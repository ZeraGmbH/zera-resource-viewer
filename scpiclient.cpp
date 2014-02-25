#include <QState>
#include <QDebug>
#include <QBuffer>
#include <protonetpeer.h>
#include <netmessages.pb.h>
#include <scpi.h>
#include "resourceviewer.h"
#include "scpiclient.h"
#include "rmprotobufwrapper.h"
ScpiClient *ScpiClient::m_pSingletonInstance = 0;

ScpiClient::ScpiClient(QObject *parent) :
    QObject(parent)
{
    m_pNetClient = 0;
    m_pScpiModel = 0;
    m_defaultWrapper = 0;
    setupStateMachine();
}

void ScpiClient::setupStateMachine()
{
    m_pStateMachine = new QStateMachine(this);
    m_pStateContainer = new QState(m_pStateMachine);
    m_pStateInit = new QState(m_pStateContainer);
    m_pStateConnected = new QState(m_pStateContainer);
    m_pStateIdentified = new QState(m_pStateContainer);
    m_pStateOperational = new QState(m_pStateContainer);
    m_pStateSendingCmd = new QState(m_pStateContainer);
    m_pFinalStateDisconnected = new QFinalState(m_pStateContainer);

    m_pStateMachine->setInitialState(m_pStateContainer);
    m_pStateContainer->setInitialState(m_pStateInit);

    m_pStateContainer->addTransition(this, SIGNAL(signalInit()), m_pStateInit);
    m_pStateConnected->addTransition(this, SIGNAL(signalIdentified()), m_pStateIdentified);
    m_pStateIdentified->addTransition(this, SIGNAL(signalOperational()), m_pStateOperational);
    m_pStateOperational->addTransition(this, SIGNAL(signalUpdateModel()), m_pStateIdentified);
    m_pStateOperational->addTransition(this, SIGNAL(signalSendSCPIMessage(QString)), m_pStateSendingCmd);
    m_pStateSendingCmd->addTransition(this, SIGNAL(signalSCPIResponse()), m_pStateOperational);

    connect(m_pStateInit, SIGNAL(entered()), this, SLOT(onInit()));
    connect(m_pStateConnected, SIGNAL(entered()), this, SLOT(onConnected()));
    connect(m_pStateIdentified, SIGNAL(entered()), this, SLOT(onIdentified()));
    connect(m_pFinalStateDisconnected, SIGNAL(entered()), this, SLOT(onDisconnected()));

    connect(this, SIGNAL(signalSendSCPIMessage(QString)), this, SLOT(slotSendSCPIMessage(QString)));
}

ScpiClient *ScpiClient::getInstance()
{
    if(m_pSingletonInstance == 0)
        m_pSingletonInstance = new ScpiClient();
    return m_pSingletonInstance;
}


void ScpiClient::onInit()
{
    m_defaultWrapper = new RMProtobufWrapper();
    m_pNetClient = new ProtoNetPeer(this);
    m_pNetClient->setWrapper(m_defaultWrapper);
    m_pStateInit->addTransition(m_pNetClient, SIGNAL(sigConnectionEstablished()), m_pStateConnected);
    m_pStateContainer->addTransition(m_pNetClient, SIGNAL(sigConnectionClosed()), m_pFinalStateDisconnected);
    m_pStateContainer->addTransition(m_pNetClient, SIGNAL(sigSocketError(QAbstractSocket::SocketError)), m_pFinalStateDisconnected);

    signalAppendLogString(tr("connecting %1:%2...").arg(m_strIPAddress).arg(m_ui16Port), LogHelper::LOG_MESSAGE);
    m_pNetClient->startConnection((m_strIPAddress), m_ui16Port);
}

void ScpiClient::onConnected()
{
    signalAppendLogString(tr("connection established"), LogHelper::LOG_MESSAGE_OK);

    connect(m_pNetClient, SIGNAL(sigMessageReceived(google::protobuf::Message*)), this, SLOT(onMessageReceived(google::protobuf::Message*)));

    signalAppendLogString(tr("sending identification..."), LogHelper::LOG_MESSAGE);
    ProtobufMessage::NetMessage envelope;
    ProtobufMessage::NetMessage::NetReply* newMessage = envelope.mutable_reply();
    newMessage->set_rtype(ProtobufMessage::NetMessage::NetReply::IDENT);
    /** @todo make id a setting
     */
    newMessage->set_body("resource-viewer");
    m_pNetClient->sendMessage(&envelope);
}

void ScpiClient::onIdentified()
{
    sendSCPIMessage(QString("resource:model?"));
    signalAppendLogString(tr("retrieving SCPI-model..."), LogHelper::LOG_MESSAGE);
}

void ScpiClient::onDisconnected()
{
    signalAppendLogString(tr("connection lost"), LogHelper::LOG_MESSAGE_ERROR);

    m_pNetClient->deleteLater();
    signalModelDeleted();
    if(m_pScpiModel != 0)
    {
        delete m_pScpiModel;
        m_pScpiModel = 0;
    }
}

void ScpiClient::onMessageReceived(google::protobuf::Message *message)
{
    ProtobufMessage::NetMessage *protoMessage = static_cast<ProtobufMessage::NetMessage *>(message);
    if(protoMessage)
    {
        ProtobufMessage::NetMessage::NetReply *reply = protoMessage->mutable_reply();
        QString strResponse = QString("%1").arg(reply->body().c_str());
        switch(reply->rtype())
        {
        case ProtobufMessage::NetMessage::NetReply::ACK:
          {
            if(m_pStateMachine->configuration().contains(m_pStateConnected))
            {
                signalAppendLogString(tr("identification acknowledged"), LogHelper::LOG_MESSAGE_OK);
                signalIdentified();
            }
            else if(m_pStateMachine->configuration().contains(m_pStateIdentified))
            {
                if(m_pScpiModel != 0)
                {
                    signalModelDeleted();
                    delete m_pScpiModel;
                    m_pScpiModel = 0;
                }
                m_pScpiModel = new cSCPI("SCPIClient");
                QByteArray tmpArr;
                QBuffer buff;
                tmpArr.append(strResponse);
                buff.setData(tmpArr);
                if(m_pScpiModel->importSCPIModelXML(&buff))
                {
                    signalAppendLogString(tr("valid model received"), LogHelper::LOG_MESSAGE_OK);
                    signalAppendLogString("", LogHelper::LOG_NEWLINE);
                    signalModelAvailable(m_pScpiModel->getSCPIModel());
                    signalOperational();
                }
                else
                    signalAppendLogString(tr("invalid model received"), LogHelper::LOG_MESSAGE_ERROR);
            }
            else if(m_pStateMachine->configuration().contains(m_pStateSendingCmd))
            {
                // notify our state machine
                signalSCPIResponse();
                // log
                signalAppendLogString(tr("SCPI in: ") + strResponse, LogHelper::LOG_MESSAGE_OK);
            }
            else
            {
                qDebug() << "SCPI answer on unkown state: "
                         << m_pStateMachine->configuration()
                         << "Answer: "
                         << reply->body().c_str();
            }
            break;
          }
        case ProtobufMessage::NetMessage::NetReply::NACK:
        {
          if(m_pStateMachine->configuration().contains(m_pStateSendingCmd))
          {
              // notify our state machine
              signalSCPIResponse();
              // log
              signalAppendLogString(tr("SCPI in (NACK): ") + strResponse, LogHelper::LOG_MESSAGE_ERROR);
          }
          break;
        }
        case ProtobufMessage::NetMessage::NetReply::ERROR:
          {
            if(m_pStateMachine->configuration().contains(m_pStateSendingCmd))
            {
                // notify our state machine
                signalSCPIResponse();
                // log
                signalAppendLogString(tr("SCPI in (ERROR): ") + strResponse, LogHelper::LOG_MESSAGE_ERROR);
            }
            break;
          }
        default:
          {
            qWarning("Unhandled answer recieved!");
            /// @todo this is the error case
            break;
          }
        }
    }
    else
        qDebug() << "protobuf read message failed";
}

void ScpiClient::start(QString strIPAddress, quint16 ui16Port)
{
    m_strIPAddress = strIPAddress;
    m_ui16Port = ui16Port;

    m_pStateMachine->start();
}

void ScpiClient::sendSCPIMessage(QString strCmd)
{
    ProtobufMessage::NetMessage envelope;
    ProtobufMessage::NetMessage::ScpiCommand* newMessage = envelope.mutable_scpi();
    newMessage->set_command(strCmd.toStdString());
    m_pNetClient->sendMessage(&envelope);
}


void ScpiClient::slotSendSCPIMessage(QString strCmd)
{
    signalAppendLogString(tr("SCPI out: ") + strCmd, LogHelper::LOG_MESSAGE);
    sendSCPIMessage(strCmd);
}

#include "scpiclient.h"
#include <QState>
#include <QDebug>
#include <QBuffer>
#include <zeraclientnetbase.h>
#include <netmessages.pb.h>
#include <scpi.h>

ScpiClient *ScpiClient::m_pSingletonInstance = 0;

ScpiClient::ScpiClient(QObject *parent) :
    QObject(parent)
{
    m_pNetClient = 0;
    m_pScpiModel = 0;

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
    m_pStateSendingCmd->addTransition(this, SIGNAL(signalSCPIResponse(QString,bool)), m_pStateOperational);

    connect(m_pStateInit, SIGNAL(entered()), this, SLOT(onInit()));
    connect(m_pStateConnected, SIGNAL(entered()), this, SLOT(onConnected()));
    connect(m_pStateIdentified, SIGNAL(entered()), this, SLOT(onIdentified()));
    connect(m_pFinalStateDisconnected, SIGNAL(entered()), this, SLOT(onDisconnected()));

    connect(this, SIGNAL(signalSendSCPIMessage(QString)), this, SLOT(sendSCPIMessage(QString)));
}

ScpiClient *ScpiClient::getInstance()
{
    if(m_pSingletonInstance == 0)
        m_pSingletonInstance = new ScpiClient();
    return m_pSingletonInstance;
}


void ScpiClient::onInit()
{
    m_pNetClient = new Zera::NetClient::cClientNetBase(this);
    m_pStateInit->addTransition(m_pNetClient, SIGNAL(connected()), m_pStateConnected);
    m_pStateContainer->addTransition(m_pNetClient, SIGNAL(connectionLost()), m_pFinalStateDisconnected);
    m_pStateContainer->addTransition(m_pNetClient, SIGNAL(tcpError(QAbstractSocket::SocketError)), m_pFinalStateDisconnected);

    m_pNetClient->startNetwork(m_strIPAddress, m_ui16Port);
}

void ScpiClient::onConnected()
{
    qDebug() << "Connected";

    ProtobufMessage::NetMessage envelope;
    ProtobufMessage::NetMessage::NetReply* newMessage = envelope.mutable_reply();
    newMessage->set_rtype(ProtobufMessage::NetMessage::NetReply::IDENT);
    newMessage->set_body("resource-viewer");
    connect(m_pNetClient, SIGNAL(messageAvailable(QByteArray)), this, SLOT(onMessageReceived(QByteArray)));

    m_pNetClient->sendMessage(&envelope);
}

void ScpiClient::onIdentified()
{
    qDebug() << "Identified";
    sendSCPIMessage(QString("resource:model?"));
}

void ScpiClient::onDisconnected()
{
    qDebug() << "Disconnected" << QObject::sender();

    m_pNetClient->deleteLater();
    signalModelDeleted();
    if(m_pScpiModel != 0)
        delete m_pScpiModel;
}

void ScpiClient::onMessageReceived(QByteArray message)
{
    ProtobufMessage::NetMessage protoMessage;
    if(Zera::NetClient::cClientNetBase::readMessage(&protoMessage, message))
    {
        ProtobufMessage::NetMessage::NetReply *reply = protoMessage.mutable_reply();
        QString strResponse = QString("%1").arg(reply->body().c_str());
        switch(reply->rtype())
        {
        case ProtobufMessage::NetMessage::NetReply::ACK:
          {
            if(m_pStateMachine->configuration().contains(m_pStateConnected))
            {
                signalIdentified();
            }
            else if(m_pStateMachine->configuration().contains(m_pStateIdentified))
            {
                qDebug() << "Model: " << strResponse;

                if(m_pScpiModel != 0)
                {
                    signalModelDeleted();
                    delete m_pScpiModel;
                }
                m_pScpiModel = new cSCPI("SCPIClient");
                QByteArray tmpArr;
                QBuffer buff;
                tmpArr.append(strResponse);
                buff.setData(tmpArr);
                m_pScpiModel->importSCPIModelXML(&buff);
                signalModelAvailable(m_pScpiModel->getSCPIModel());
                signalOperational();
            }
            else if(m_pStateMachine->configuration().contains(m_pStateSendingCmd))
            {
                qDebug() << "SCPI answer: " << reply->body().c_str();
                signalSCPIResponse(strResponse, true);
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
        case ProtobufMessage::NetMessage::NetReply::ERROR:
          {
            if(m_pStateMachine->configuration().contains(m_pStateSendingCmd))
            {
                signalSCPIResponse(strResponse, false);
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

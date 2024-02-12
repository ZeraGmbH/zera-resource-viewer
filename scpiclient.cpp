#include <QState>
#include <QDebug>
#include <QBuffer>
#include <QDomDocument>
#include <QDomDocumentType>
#include <QDomElement>
#include <vtcp_peer.h>
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

    m_pStateContainer->addTransition(this, &ScpiClient::signalInit, m_pStateInit);
    m_pStateConnected->addTransition(this, &ScpiClient::signalIdentified, m_pStateIdentified);
    m_pStateIdentified->addTransition(this, &ScpiClient::signalOperational, m_pStateOperational);
    m_pStateOperational->addTransition(this, &ScpiClient::signalUpdateModel, m_pStateIdentified);
    m_pStateOperational->addTransition(this, &ScpiClient::signalSendSCPIMessage, m_pStateSendingCmd);
    m_pStateSendingCmd->addTransition(this, &ScpiClient::signalSCPIResponse, m_pStateOperational);

    connect(m_pStateInit, &QState::entered, this, &ScpiClient::onInit);
    connect(m_pStateConnected, &QState::entered, this, &ScpiClient::onConnected);
    connect(m_pStateIdentified, &QState::entered, this, &ScpiClient::onIdentified);
    connect(m_pFinalStateDisconnected, &QState::entered, this, &ScpiClient::onDisconnected);

    connect(this, &ScpiClient::signalSendSCPIMessage, this, &ScpiClient::slotSendSCPIMessage);
}

ScpiClient *ScpiClient::getInstance()
{
    if(m_pSingletonInstance == 0) {
        m_pSingletonInstance = new ScpiClient();
    }
    return m_pSingletonInstance;
}


void ScpiClient::onInit()
{
    m_defaultWrapper = new RMProtobufWrapper();
    m_pNetClient = new VeinTcp::TcpPeer(this);
    m_pStateInit->addTransition(m_pNetClient, &VeinTcp::TcpPeer::sigConnectionEstablished, m_pStateConnected);
    m_pStateContainer->addTransition(m_pNetClient, &VeinTcp::TcpPeer::sigConnectionClosed, m_pFinalStateDisconnected);
    m_pStateContainer->addTransition(m_pNetClient, &VeinTcp::TcpPeer::sigSocketError, m_pFinalStateDisconnected);

    emit signalAppendLogString(tr("connecting %1:%2...").arg(m_strIPAddress).arg(m_ui16Port), LogHelper::LOG_MESSAGE);
    m_pNetClient->startConnection((m_strIPAddress), m_ui16Port);
}

void ScpiClient::onConnected()
{
    emit signalAppendLogString(tr("connection established"), LogHelper::LOG_MESSAGE_OK);

    connect(m_pNetClient, &VeinTcp::TcpPeer::sigMessageReceived, this, &ScpiClient::onMessageReceived);

    emit signalAppendLogString(tr("sending identification..."), LogHelper::LOG_MESSAGE);
    ProtobufMessage::NetMessage envelope;
    ProtobufMessage::NetMessage::NetReply* newMessage = envelope.mutable_reply();
    newMessage->set_rtype(ProtobufMessage::NetMessage::NetReply::IDENT);
    /** @todo make id a setting
     */
    newMessage->set_body("resource-viewer");
    m_pNetClient->sendMessage(m_defaultWrapper->protobufToByteArray(envelope));
}

void ScpiClient::onIdentified()
{
    sendSCPIMessage(QString("resource:model?"));
    emit signalAppendLogString(tr("retrieving SCPI-model..."), LogHelper::LOG_MESSAGE);
}

void ScpiClient::onMessageReceived(VeinTcp::TcpPeer *peer, QByteArray message)
{
    Q_UNUSED(peer)
    handleMessageReceived(m_defaultWrapper->byteArrayToProtobuf(message));
}

void ScpiClient::onDisconnected()
{
    emit signalAppendLogString(tr("connection lost"), LogHelper::LOG_MESSAGE_ERROR);

    m_pNetClient->deleteLater();
    emit signalModelDeleted();
    if(m_pScpiModel != 0) {
        delete m_pScpiModel;
        m_pScpiModel = 0;
    }
}


void ScpiClient::handleMessageReceived(std::shared_ptr<google::protobuf::Message> message)
{
    std::shared_ptr<ProtobufMessage::NetMessage> protoMessage = std::static_pointer_cast<ProtobufMessage::NetMessage>(message);
    if(protoMessage) {
        ProtobufMessage::NetMessage::NetReply *reply = protoMessage->mutable_reply();
        QString strResponse = QString("%1").arg(reply->body().c_str());
        switch(reply->rtype()) {
        case ProtobufMessage::NetMessage::NetReply::ACK:
            if(m_pStateMachine->configuration().contains(m_pStateConnected)) {
                emit signalAppendLogString(tr("identification acknowledged"), LogHelper::LOG_MESSAGE_OK);
                emit signalIdentified();
            }
            else if(m_pStateMachine->configuration().contains(m_pStateIdentified))
            {
                if(m_pScpiModel != 0) {
                    emit signalModelDeleted();
                    delete m_pScpiModel;
                    m_pScpiModel = 0;
                }
                m_pScpiModel = new QStandardItemModel;
                QByteArray tmpArr;
                QBuffer buff;
                tmpArr.append(strResponse.toUtf8());
                buff.setData(tmpArr);
                if(importSCPIModelXML(&buff)) {
                    emit signalAppendLogString(tr("valid model received"), LogHelper::LOG_MESSAGE_OK);
                    emit signalAppendLogString("", LogHelper::LOG_NEWLINE);
                    emit signalModelAvailable(m_pScpiModel);
                    emit signalOperational();
                }
                else {
                    emit signalAppendLogString(tr("invalid model received"), LogHelper::LOG_MESSAGE_ERROR);
                }
            }
            else if(m_pStateMachine->configuration().contains(m_pStateSendingCmd)) {
                // notify our state machine
                emit signalSCPIResponse();
                // log
                emit signalAppendLogString(tr("SCPI in: ") + strResponse, LogHelper::LOG_MESSAGE_OK);
            }
            else {
                qDebug() << "SCPI answer on unkown state: "
                         << m_pStateMachine->configuration()
                         << "Answer: "
                         << reply->body().c_str();
            }
            break;
        case ProtobufMessage::NetMessage::NetReply::NACK:
            if(m_pStateMachine->configuration().contains(m_pStateSendingCmd)) {
                // notify our state machine
                emit signalSCPIResponse();
                // log
                emit signalAppendLogString(tr("SCPI in (NACK): ") + strResponse, LogHelper::LOG_MESSAGE_ERROR);
            }
            break;
        case ProtobufMessage::NetMessage::NetReply::ERROR:
            if(m_pStateMachine->configuration().contains(m_pStateSendingCmd)) {
                // notify our state machine
                emit signalSCPIResponse();
                // log
                emit signalAppendLogString(tr("SCPI in (ERROR): ") + strResponse, LogHelper::LOG_MESSAGE_ERROR);
            }
            break;
        default:
            qWarning("Unhandled answer recieved!");
            /// @todo this is the error case
            break;
        }
    }
    else {
        qWarning() << "protobuf read message failed";
    }
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
    m_pNetClient->sendMessage(m_defaultWrapper->protobufToByteArray(envelope));
}


void ScpiClient::slotSendSCPIMessage(QString strCmd)
{
    emit signalAppendLogString(tr("SCPI out: ") + strCmd, LogHelper::LOG_MESSAGE);
    sendSCPIMessage(strCmd);
}

void ScpiClient::genSCPICmd(const QStringList&  parentnodeNames, cSCPINode* pSCPINode)
{
    QStringList::const_iterator it;
    QStandardItem *parentItem;
    QStandardItem *childItem;
    QModelIndex childModelIndex;

    parentItem = m_pScpiModel->invisibleRootItem();

    for (it = parentnodeNames.begin(); it != parentnodeNames.end(); ++it)
    {
            childItem = 0;
            quint32 nrows = parentItem->rowCount();

            if (nrows > 0)
                for (quint32 i = 0; i < nrows; i++)
                {
                    childItem = parentItem->child(i);
                    if (childItem->data(Qt::DisplayRole) == *it)
                        break;
                    else
                        childItem = 0;
                }

            if (!childItem)
            {
                childItem  = new cSCPINode(*it, SCPI::isNode, 0);
                parentItem->appendRow(childItem);
            }

            parentItem = childItem;
    }

    parentItem->appendRow(pSCPINode);
}

quint8 ScpiClient::getNodeType(const QString& sAttr)
{
    int t = SCPI::isNode;
    if ( sAttr.contains(scpiNodeType[SCPI::Query]) )
        t += SCPI::isQuery;
    if ( sAttr.contains(scpiNodeType[SCPI::CmdwP]) )
        t += SCPI::isCmdwP;
    else
    if ( sAttr.contains(scpiNodeType[SCPI::Cmd]) )
        t += SCPI::isCmd;

    return t;
}

bool ScpiClient::getcommandInfo( QDomNode rootNode, quint32 nlevel )
{
    static QStringList nodeNames;
    QDomNodeList nl = rootNode.childNodes();

    quint32 n = nl.length();
    if ( n > 0)
    {
        quint32 i;
        for (i=0; i<n ; i++)
        {
            if (nlevel == 0)
                nodeNames.clear(); // for each root cmd we clear the local node name list
            QDomNode node2 = nl.item(i);
            QDomElement e = node2.toElement();
            QString s = e.tagName();
            if (!node2.hasChildNodes())
            {
                quint8 t = getNodeType(e.attribute(scpinodeAttributeName));
                cSCPINode *pSCPINode = new cSCPINode (s , t, NULL); // we have no corresponding cSCPIObject -> NULL
                genSCPICmd(nodeNames, pSCPINode);
            }
            else
            {
                nodeNames.append(s); // we add each node name to the list
                getcommandInfo(node2, ++nlevel); // we look for further levels
                --nlevel;
            }
        }
    }

    if (nlevel > 0)
        nodeNames.pop_back();

    return true;
}

void ScpiClient::clearSCPICmdList()
{
    QList<QStandardItem *> itemList;

    itemList = m_pScpiModel->takeColumn (0);
    for (qint32 i = 0; i < itemList.size(); ++i)
        delete itemList.value(i);
}

bool ScpiClient::importSCPIModelXML(QIODevice *ioDevice)
{
    clearSCPICmdList();

    QDomDocument modelDoc;
    if ( !modelDoc.setContent(ioDevice) )
        return false;

    QDomDocumentType TheDocType = modelDoc.doctype ();
    if (TheDocType.name() != scpimodelDocName)
        return false;

    QDomElement rootElem = modelDoc.documentElement();
    if (rootElem.tagName() != scpimodelrootName)
        return false;

    QDomNodeList nl=rootElem.childNodes();

    for (int i=0; i<nl.length() ; i++)
    {
        QDomNode n = nl.item(i);
        QDomElement e=n.toElement();

        if (e.tagName() == scpimodelsTag) // here the scpi models start
        {
            m_pScpiModel->clear(); // we clear the existing command list
            if (!getcommandInfo( n, 0))
                return false;
        }

    }

    return true;
}

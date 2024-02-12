#ifndef SCPICLIENT_H
#define SCPICLIENT_H

#include <QObject>
#include <QStandardItemModel>
#include <QStateMachine>
#include <QFinalState>
#include <QString>
#include <QDomNode>
#include "scpinode.h"
#include "loghelper.h"

class XiQNetPeer;
class cSCPI;
class RMProtobufWrapper;

namespace google
{
  namespace protobuf
  {
    class Message;
  }
}

const QString scpimodelDocName = "SCPIModel";
const QString scpimodelrootName = "MODELLIST";
const QString scpimodeldeviceTag =  "DEVICE";
const QString scpimodelsTag = "MODELS";
const QString scpinodeAttributeName = "Type";

class ScpiClient : public QObject
{
    Q_OBJECT
public:
    static ScpiClient* getInstance();
    void start(QString strIPAddress, quint16 ui16Port);

signals:
    void signalInit();
    void signalIdentified();
    void signalOperational();
    void signalUpdateModel();
    void signalModelAvailable(QStandardItemModel* newModel);
    void signalModelDeleted();
    void signalSendSCPIMessage(QString strCmd);
    void signalSCPIResponse();
    void signalAppendLogString(QString strMsg, LogHelper::enLogTypes eType);
public slots:
    void slotSendSCPIMessage(QString strCmd);

private slots:
    void onInit();
    void onConnected();
    void onIdentified();
    void onMessageReceived(XiQNetPeer *peer, QByteArray message);
    void onDisconnected();

private:
    explicit ScpiClient(QObject *parent = 0);
    void setupStateMachine();
    void sendSCPIMessage(QString strCmd);
    void genSCPICmd(const QStringList&  parentnodeNames, cSCPINode* pSCPINode);
    quint8 getNodeType(const QString& sAttr);
    bool getcommandInfo(QDomNode rootNode, quint32 nlevel);
    void clearSCPICmdList();
    bool importSCPIModelXML(QIODevice *ioDevice);
    void handleMessageReceived(std::shared_ptr<google::protobuf::Message> message);

    static ScpiClient* m_pSingletonInstance;
    QString m_strIPAddress;
    quint16 m_ui16Port;

    QStateMachine *m_pStateMachine;
    QState *m_pStateContainer;
    QState *m_pStateInit;
    QState *m_pStateConnected;
    QState *m_pStateIdentified;
    QState *m_pStateOperational;
    QState *m_pStateSendingCmd;
    QFinalState *m_pFinalStateDisconnected;
    XiQNetPeer *m_pNetClient;
    QStandardItemModel *m_pScpiModel;
    RMProtobufWrapper *m_defaultWrapper;
};


#endif // SCPICLIENT_H

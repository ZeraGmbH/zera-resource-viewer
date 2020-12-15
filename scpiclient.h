#ifndef SCPICLIENT_H
#define SCPICLIENT_H

#include <QObject>
#include <QStandardItemModel>
#include <QStateMachine>
#include <QFinalState>
#include <QString>
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


class ScpiClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Get singleton instance of ScpiClient
     */
    static ScpiClient* getInstance();
    /**
     * @brief Starts our state machine to establish connection to server.
     * @param strIPAddress Server's IP address or host name
     * @param ui16Port The port number our server is listening on.
     */
    void start(QString strIPAddress, quint16 ui16Port);

signals:
    /**
     * @brief Initial state for our state machine connected to onInit().
     */
    void signalInit();

    /**
     * @brief When an identification was responded with ACK,
     *  this signal is thrown causing state machine pass to state ScpiClient::m_pStateIdentified.
     */
    void signalIdentified();

    /**
     * @brief On receiving a proper SCPI-model this signal is thrown.
     *  It causes ScpiClient::m_pStateMachine to move to state ScpiClient::m_pStateOperational
     */
    void signalOperational();

    /**
     * @brief To reload the SCPI-model this signal must be fired.
     *  It causes ScpiClient::m_pStateMachine to move to state ScpiClient::m_pStateIdentified which asks server for a new model.
     */
    void signalUpdateModel();

    /**
     * @brief This signal is thrown on receiving a new model from our server to inform our displaying client.
     * @param newModel
     */
    void signalModelAvailable(QStandardItemModel* newModel);
    /**
     * @brief This signal is thrown either on network disconnect or in update-model-cycle.
     */
    void signalModelDeleted();

    /**
     * @brief Call this signal to send a SCPI command.
     * @param strCmd Formatted SCPI-command
     */
    void signalSendSCPIMessage(QString strCmd);
    /**
     * @brief Notify our state machine for Server's SCPI command response
     */
    void signalSCPIResponse();
    /**
     * @brief Signal to append a log string
     * @param strMsg Message to log
     * @param eType Type of message
     */
    void signalAppendLogString(QString strMsg, LogHelper::enLogTypes eType);
public slots:
    /**
     * @brief To send a SCPI command this slot can be called directly or by signal signalSendSCPIMessage().
     * @param strCmd SCPI command to send
     */
    void slotSendSCPIMessage(QString strCmd);

private slots:
    /**
     * @brief ScpiClient::m_pNetClient is created here, the transitions caused by network connection
     *  are set up and network connection to our server is started.
     */
    void onInit();
    /**
     * @brief Network connection was established so we send our identification to the server.
     */
    void onConnected();
    /**
     * @brief Our Identification was accepted. We ask the server for SCPI-model here.
     */
    void onIdentified();
    /**
     * @brief ScpiClient::m_pNetClient is destroyed and signalModelDeleted() is emitted.
     */
    void onDisconnected();
    /**
     * @brief This slot is connected to messageAvailable of ScpiClient::m_pNetClient
     * @param message contains the ProtobufMessage
     */
    void onMessageReceived(google::protobuf::Message *message);


private:
    /**
     * @brief private constructor to avoid multiple instances
     * @param parent
     */
    explicit ScpiClient(QObject *parent = 0);
    /**
     * @brief All states and transitions (except those linked to ScpiClient::m_pNetClient) are established here
     */
    void setupStateMachine();
    /**
     * @brief To send a SCPI command we call this function.
     * @param strCmd
     */
    void sendSCPIMessage(QString strCmd);

    /**
     * @brief Our one and only instance.
     */
    static ScpiClient* m_pSingletonInstance;

    /**
     * @brief Keeper for IP address received in start().
     */
    QString m_strIPAddress;
    /**
     * @brief Keeper for IP port received in start().
     */
    quint16 m_ui16Port;

    /**
     * @brief Our state machine with states below
     */
    QStateMachine *m_pStateMachine;
    /**
     * @brief Container for all other states.
     *  It was introduced to add transitions for all states to ScpiClient::m_pFinalStateDisconnected in case
     *  network goes down.
     */
    QState *m_pStateContainer;
    /**
     * @brief Initial state connected to onInit() slot.
     */
    QState *m_pStateInit;
    /**
     * @brief This state is entered when ScpiClient::m_pNetClient fires connected-signal.
     *  'entered'-signal is connected to onConnected() slot.
     */
    QState *m_pStateConnected;
    /**
     * @brief This state is entered when our server accepts the identification sent.
     *  'entered'-signal is connected to onIdentified() slot.
     */
    QState *m_pStateIdentified;
    /**
     * @brief In this state a valid model was received.
     *  It has transitions to ScpiClient::m_pStateSendingCmd on signalSendSCPIMessage() and ScpiClient::m_pStateIdentified on signalUpdateModel()
     */
    QState *m_pStateOperational;
    /**
     * @brief ScpiClient::m_pStateMachine enters this state while a SCPI-command is pending.
     */
    QState *m_pStateSendingCmd;
    /**
     * @brief In case connection to server goes down ScpiClient::m_pStateMachine reaches it't final state.
     *  'entered'-signal is connected to onDisconnected() slot.
     */
    QFinalState *m_pFinalStateDisconnected;

    /**
     * @brief Our handler for network I/O. Instance is created onInit() and destroyed onDisconnected().
     */
    XiQNetPeer *m_pNetClient;

    cSCPI *m_pScpiModel;

    RMProtobufWrapper *m_defaultWrapper;
};


#endif // SCPICLIENT_H

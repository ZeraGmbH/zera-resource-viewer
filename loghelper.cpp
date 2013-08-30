#include <QTime>
#include <QString>
#include "loghelper.h"


QString LogHelper::formatMessage(QString strMsg, enum enLogTypes logType)
{
    // add time
    QTime currentTime = QTime::currentTime();
    strMsg = currentTime.toString("HH:mm:ss.zzz: ") + strMsg;
    // add color / bold
    switch(logType)
    {
    case LOG_MESSAGE_OK:
        strMsg = "<p style=\"color:green\">" + strMsg + "</p>";
        break;
    case LOG_MESSAGE_ERROR:
        strMsg = "<b style=\"color:red\">" + strMsg + "</b>";
        break;
    case LOG_NEWLINE:
        // ignore incoming strMsg
        strMsg ="";
        break;
    default:
        break;
    }
    return strMsg;
}

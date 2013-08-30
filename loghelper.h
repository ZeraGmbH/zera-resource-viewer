#ifndef LOGHELPER_H
#define LOGHELPER_H

class LogHelper
{
public:
    enum enLogTypes
    {
        LOG_MESSAGE,
        LOG_MESSAGE_OK,
        LOG_MESSAGE_ERROR,
        LOG_NEWLINE
    };

    static QString formatMessage(QString strMsg, enum enLogTypes logType);
};

#endif // LOGHELPER_H

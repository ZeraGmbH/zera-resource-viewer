#ifndef PROTOBUFWRAPPER_H
#define PROTOBUFWRAPPER_H

#include <QByteArray>
#include <memory>
#include <google/protobuf/message.h>

class RMProtobufWrapper
{
public:
    static std::shared_ptr<google::protobuf::Message> byteArrayToProtoRm(const QByteArray &byteArray);
};

#endif // PROTOBUFWRAPPER_H

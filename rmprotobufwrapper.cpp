#include "rmprotobufwrapper.h"

#include <netmessages.pb.h>
#include <QDebug>

RMProtobufWrapper::RMProtobufWrapper()
{
}

std::shared_ptr<google::protobuf::Message> RMProtobufWrapper::byteArrayToProtobuf(const QByteArray &byteArray)
{
    ProtobufMessage::NetMessage *intermediate = new ProtobufMessage::NetMessage();
    if(!intermediate->ParseFromArray(byteArray, byteArray.size())) {
        qCritical() << "Error parsing protobuf:\n" << byteArray.toBase64();
        Q_ASSERT(false);
    }
    std::shared_ptr<google::protobuf::Message> proto {intermediate};
    return proto;
}

QByteArray RMProtobufWrapper::protobufToByteArray(const google::protobuf::Message &protobufMessage)
{
    return QByteArray(protobufMessage.SerializeAsString().c_str(), protobufMessage.ByteSizeLong());
}

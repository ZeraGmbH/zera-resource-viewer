#include "rmprotobufwrapper.h"

#include <netmessages.pb.h>
#include <QDebug>

RMProtobufWrapper::RMProtobufWrapper()
{
}

std::shared_ptr<google::protobuf::Message> RMProtobufWrapper::byteArrayToProtobuf(QByteArray bA)
{
    ProtobufMessage::NetMessage *intermediate = new ProtobufMessage::NetMessage();
    if(!intermediate->ParseFromArray(bA, bA.size())) {
        qCritical() << "Error parsing protobuf:\n" << bA.toBase64();
        Q_ASSERT(false);
    }
    std::shared_ptr<google::protobuf::Message> proto {intermediate};
    return proto;
}

QByteArray RMProtobufWrapper::protobufToByteArray(const google::protobuf::Message &pMessage)
{
    return QByteArray(pMessage.SerializeAsString().c_str(), pMessage.ByteSizeLong());
}

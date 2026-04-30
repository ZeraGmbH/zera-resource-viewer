#include "rmprotobufwrapper.h"

#include <netmessages.pb.h>
#include <QDebug>

std::shared_ptr<google::protobuf::Message> RMProtobufWrapper::byteArrayToProtoRm(const QByteArray &byteArray)
{
    ProtobufMessage::NetMessage *intermediate = new ProtobufMessage::NetMessage();
    if(!intermediate->ParseFromArray(byteArray, byteArray.size())) {
        qCritical() << "Error parsing protobuf:\n" << byteArray.toBase64();
        Q_ASSERT(false);
    }
    std::shared_ptr<google::protobuf::Message> proto {intermediate};
    return proto;
}

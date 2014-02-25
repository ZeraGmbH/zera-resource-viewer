#include "rmprotobufwrapper.h"

#include <netmessages.pb.h>
#include <QDebug>

RMProtobufWrapper::RMProtobufWrapper()
{
}


google::protobuf::Message *RMProtobufWrapper::byteArrayToProtobuf(QByteArray bA)
{
  ProtobufMessage::NetMessage *proto = new ProtobufMessage::NetMessage();
  if(!proto->ParseFromArray(bA, bA.size()))
  {
    qCritical() << "Error parsing protobuf:\n" << bA.toBase64();
    Q_ASSERT(false);
  }
  return proto;
}

QByteArray RMProtobufWrapper::protobufToByteArray(google::protobuf::Message *pMessage)
{
  return QByteArray(pMessage->SerializeAsString().c_str(), pMessage->ByteSize());
}

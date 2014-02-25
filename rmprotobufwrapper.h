#ifndef PROTOBUFWRAPPER_H
#define PROTOBUFWRAPPER_H

#include <protonetwrapper.h>

class RMProtobufWrapper : public ProtoNetWrapper
{
public:
  RMProtobufWrapper();


  google::protobuf::Message *byteArrayToProtobuf(QByteArray bA); //override;

  QByteArray protobufToByteArray(google::protobuf::Message *pMessage); //override;
};

#endif // PROTOBUFWRAPPER_H

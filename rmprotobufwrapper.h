#ifndef PROTOBUFWRAPPER_H
#define PROTOBUFWRAPPER_H

#include <xiqnetwrapper.h>

class RMProtobufWrapper : public XiQNetWrapper
{
public:
  RMProtobufWrapper();

  virtual std::shared_ptr<google::protobuf::Message> byteArrayToProtobuf(const QByteArray &byteArray) override;
  virtual QByteArray protobufToByteArray(const google::protobuf::Message &protobufMessage) override;
};

#endif // PROTOBUFWRAPPER_H

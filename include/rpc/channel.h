#ifndef __FZ_RPC_CHANNEL_H__
#define __FZ_RPC_CHANNEL_H__

#include <google/protobuf/service.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "net/common/buffer.h"
#include "net/session.h"
#include "rpc/codec.h"

namespace fz::rpc {

class Channel : public google::protobuf::RpcChannel {
 public:
  Channel(std::weak_ptr<net::Session> session,
          std::shared_ptr<
              std::unordered_map<std::string, google::protobuf::Service*>>
              services)
      : _session{std::move(session)}, _services{std::move(services)} {}

  explicit Channel(std::weak_ptr<net::Session> session)
      : _session{std::move(session)} {}

 public:
  auto CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done) -> void override;

  auto readCallback(const std::shared_ptr<net::Session>& session,
                    net::Buffer& buffer) -> void;

 private:
  auto onRequestMessage() -> void;

  auto onResponseMessage() -> void;

  // while the server is done with the request, it will call this function
  auto onDoneRequest(std::uint64_t message_id,
                     ::google::protobuf::Message* response) -> void;

 private:
  std::weak_ptr<net::Session> _session;
  std::shared_ptr<std::unordered_map<std::string, google::protobuf::Service*>>
      _services;
  std::mutex _mutex;
  Codec _codec;
  struct SentRpcRequest {
    google::protobuf::Message* _responses;
    google::protobuf::Closure* _closures;
  };
  std::unordered_map<std::uint64_t, SentRpcRequest> _sent_rpcs;
};

}  // namespace fz::rpc

#endif  // __FZ_RPC_CHANNEL_H__

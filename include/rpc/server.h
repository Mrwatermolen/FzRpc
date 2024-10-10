#ifndef __FZ_RPC_SERVER_H__
#define __FZ_RPC_SERVER_H__

#include <google/protobuf/service.h>

#include <cstddef>
#include <memory>
#include <unordered_map>

#include "net/tcp_server.h"

namespace fz::rpc {

class Server {
 public:
  Server(std::size_t num_thread, std::string_view ip, uint16_t port);

  auto start() -> void;

  auto stop() -> void;

  auto registerService(::google::protobuf::Service* service) -> void;

 private:
  fz::net::TcpServer _tcp_server;
  std::shared_ptr<std::unordered_map<std::string, ::google::protobuf::Service*>>
      _services;
};

}  // namespace fz::rpc

#endif  // __FZ_RPC_SERVER_H__

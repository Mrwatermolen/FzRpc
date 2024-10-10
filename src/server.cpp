#include "rpc/server.h"

#include "rpc/session.h"

namespace fz::rpc {

Server::Server(std::size_t num_thread, std::string_view ip, uint16_t port)
    : _tcp_server{num_thread, ip, port},
      _services{std::make_shared<
          std::unordered_map<std::string, ::google::protobuf::Service*>>()} {
  _tcp_server.setNewSessionCallback<fz::rpc::Session>();
  _tcp_server.setConnectCallback(
      [this](const std::shared_ptr<fz::net::Session>& session) {
        auto rpc_session = std::dynamic_pointer_cast<fz::rpc::Session>(session);
        if (rpc_session == nullptr) {
          return;
        }
        auto rpc_channel = std::make_shared<Channel>(rpc_session, _services);
        rpc_session->setChannel(rpc_channel);
      });

  _tcp_server.setReadCallback([](const auto& session, auto&& buffer) {
    auto rpc_session = std::dynamic_pointer_cast<fz::rpc::Session>(session);
    if (rpc_session == nullptr) {
      return;
    }

    rpc_session->channel()->readCallback(session, buffer);
  });
}

auto Server::start() -> void { _tcp_server.start(); }

auto Server::stop() -> void { _tcp_server.stop(); }

auto Server::registerService(::google::protobuf::Service* service) -> void {
  auto service_name = service->GetDescriptor()->full_name();
  (*_services)[service_name] = service;
}

}  // namespace fz::rpc

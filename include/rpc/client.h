#ifndef __FZ_RPC_CLIENT_H__
#define __FZ_RPC_CLIENT_H__

#include "net/loop.h"

#include <memory>
#include <string>

#include "rpc/channel.h"
#include "rpc/session.h"

namespace fz::rpc {

class Client {
 public:
  Client(std::shared_ptr<fz::net::Loop> loop, std::string_view ip,
         std::uint16_t port)
      : _loop{loop},
        _session{std::make_shared<fz::rpc::Session>(loop)},
        _channel{std::make_shared<fz::rpc::Channel>(_session)},
        _ip{ip},
        _port{port} {
    _session->setChannel(_channel);
    _session->setReadCallback([this](const auto& session, auto& buffer) {
      _channel->readCallback(session, buffer);
    });
  }

  auto start() { _loop->start(); }

  auto connect() { _session->connect(_ip, _port, true); }

  auto stop() { _loop->stop(); }

  auto& channel() { return _channel; }

  auto& channel() const { return _channel; }

 private:
  std::shared_ptr<fz::net::Loop> _loop;
  std::shared_ptr<fz::rpc::Session> _session;
  std::shared_ptr<fz::rpc::Channel> _channel;
  std::string _ip;
  std::uint16_t _port;
};

}  // namespace fz::rpc

#endif  // __FZ_RPC_CLIENT_H__

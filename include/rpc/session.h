#ifndef __FZ_RPC_SESSION_H__
#define __FZ_RPC_SESSION_H__

#include <memory>

#include "net/session.h"
#include "rpc/channel.h"

namespace fz::rpc {

class Session : public fz::net::Session {
 public:
  explicit Session(std::shared_ptr<fz::net::Loop> loop)
      : fz::net::Session{std::move(loop)} {}

  auto& channel() { return _channel; }

  auto& channel() const { return _channel; }

  auto setChannel(std::shared_ptr<Channel> channel) -> void {
    _channel = std::move(channel);
  }

 private:
  std::shared_ptr<Channel> _channel;
};

}  // namespace fz::rpc

#endif  // __FZ_RPC_SESSION_H__

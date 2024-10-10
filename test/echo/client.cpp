#include "rpc/client.h"

#include <memory>
#include <utility>

#include "echo.pb.h"
#include "net/loop.h"

class EchoClient {
 public:
  EchoClient(std::shared_ptr<fz::net::Loop> loop, std::string_view ip,
             uint16_t port)
      : _client{std::move(loop), ip, port},
        _stub{_client.channel().get(),
              google::protobuf::Service::STUB_DOESNT_OWN_CHANNEL} {}

  auto start() -> void { _client.start(); }

  auto connect() -> void { _client.connect(); }

  auto& stub() { return _stub; }

  auto& stub() const { return _stub; }

  auto stop() -> void { _client.stop(); }

 public:
  auto requestEcho(const echo::EchoRequest& request) -> void {
    auto response = std::make_shared<echo::EchoReply>();
    _stub.requestEcho(nullptr, &request, response.get(),
                      ::google::protobuf::NewCallback(
                          this, &EchoClient::requestEchoCallBack, response));
  }

 private:
  // google::protobuf::NewCallback can't use reference
  auto requestEchoCallBack(std::shared_ptr<echo::EchoReply> response) -> void {
    std::cout << "Received response with message: " << response->message()
              << '\n';
  }

 private:
  fz::rpc::Client _client;
  echo::Echo_Stub _stub;
};

int main() {
  auto client =
      EchoClient{std::make_shared<fz::net::Loop>(), "127.0.0.1", 5501};
  client.connect();

  auto request = echo::EchoRequest{};
  request.set_message("Hello world");
  client.requestEcho(request);
  client.start();

  // Make sure main thread is running
  asio::io_context io_context;
  asio::signal_set signals{io_context, SIGINT, SIGTERM};
  signals.async_wait([&io_context, &client](const auto&, int) {
    client.stop();
    io_context.stop();
  });
  io_context.run();
  return 0;
}

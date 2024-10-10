#include "rpc/server.h"

#include <asio/io_context.hpp>

#include "echo.pb.h"

namespace echo {

class EchoImp : public echo::Echo {
 public:
  auto requestEcho(::google::protobuf::RpcController* controller,
                   const EchoRequest* request, EchoReply* response,
                   ::google::protobuf::Closure* done) -> void override {
    std::cout << "Received request with message: " << request->message()
              << '\n';
    response->set_message("Server received: " + request->message());
    done->Run();
  }

 private:
};

}  // namespace echo

int main() {
  fz::rpc::Server server(1, "127.0.0.1", 5501);
  auto echo_service = new echo::EchoImp();
  server.registerService(echo_service);
  server.start();

  auto io_context = asio::io_context();
  auto signal_set = asio::signal_set(io_context, SIGINT, SIGTERM);
  signal_set.async_wait([&](const std::error_code&, int) { server.stop(); });
  io_context.run();
  return 0;
}
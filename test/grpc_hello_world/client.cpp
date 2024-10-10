#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

#include "hello_world.grpc.pb.h"

namespace hello_world {
class HelloWorldClient {
 public:
  explicit HelloWorldClient(std::shared_ptr<grpc::Channel> channel)
      : _stub(HelloWorld::NewStub(std::move(channel))) {}

  auto sendRequest(int id, const std::string& name) {
    HelloWorldRequest request;
    request.set_id(id);
    request.set_name(name);
    HelloWorldReply response;
    grpc::ClientContext context;
    auto status = _stub->requestHelloWorld(&context, request, &response);
    if (status.ok()) {
      std::cout << "Received response with id: " << response.id()
                << " and message: " << response.message() << '\n';
    } else {
      std::cout << "RPC failed with error: " << status.error_code() << ": "
                << status.error_message() << '\n';
    }
  }

 private:
  std::unique_ptr<HelloWorld::Stub> _stub;
};
}  // namespace hello_world

int main() {
  auto channel = grpc::CreateChannel("localhost:5501",
                                     grpc::InsecureChannelCredentials());
  hello_world::HelloWorldClient client(std::move(channel));
  client.sendRequest(1, "Alice");
  return 0;
}

#include <grpcpp/grpcpp.h>

#include "hello_world.grpc.pb.h"

namespace hello_world {

class HelloWorldServiceImpl final : public HelloWorld::Service {
  auto requestHelloWorld(grpc::ServerContext* context,
                   const HelloWorldRequest* request, HelloWorldReply* response)
      -> grpc::Status override {
    auto id = request->id();
    const auto& name = request->name();
    response->set_id(id);
    std::string message = "Hello world, " + name;
    response->set_message(message);
    std::cout << "Received request with id: " << id << " and name: " << name
              << '\n';

    return grpc::Status::OK;
  }
};

}  // namespace hello_world

int main() {
  auto builder = grpc::ServerBuilder();
  builder.AddListeningPort("0.0.0.0:5501", grpc::InsecureServerCredentials());
  hello_world::HelloWorldServiceImpl service;
  builder.RegisterService(&service);
  auto server = builder.BuildAndStart();
  std::cout << "Server listening on port 5501" << '\n';
  server->Wait();
}

#include "rpc/channel.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include <cassert>
#include <exception>
#include <mutex>
#include <string>
#include <utility>

#include "net/common/log.h"
#include "rpc/codec.h"

namespace fz::rpc {

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<std::uint64_t> dis(
    0, std::numeric_limits<std::uint64_t>::max());

// make random id
static auto makeMessageId() -> std::uint64_t { return dis(gen); }

auto Channel::CallMethod(const google::protobuf::MethodDescriptor* method,
                         google::protobuf::RpcController* controller,
                         const google::protobuf::Message* request,
                         google::protobuf::Message* response,
                         google::protobuf::Closure* done) -> void {
  auto id = makeMessageId();
  auto buffer = Codec::makeMessageBuffer(
      id, true, method->service()->full_name() + "::" + method->name(),
      *request);

  {
    std::scoped_lock lock(_mutex);
    if (_sent_rpcs.find(id) != _sent_rpcs.end()) {
      LOG_CRITICAL("ID: {} already exists.", id);
      std::terminate();
    }

    _sent_rpcs[id] = {._responses = response, ._closures = done};
  }

  LOG_DEBUG("ID: {}. FullName: {}", id,
            method->service()->full_name() + "::" + method->name());
  _session.lock()->send(buffer);
}

auto Channel::readCallback(const std::shared_ptr<net::Session>& session,
                           net::Buffer& buffer) -> void {
  _codec.read(buffer);

  if (_codec.state() == Codec::State::Invalid) {
    LOG_ERROR("Invalid codec state.{}", "");
    _codec.reset();
  }

  if (_codec.state() != Codec::State::OK) {
    return;
  }

  if (_codec.isRequest()) {
    onRequestMessage();
  } else {
    onResponseMessage();
  }

  _codec.reset();
}

auto Channel::onRequestMessage() -> void {
  LOG_DEBUG("message id: {}, full name: {}", _codec.messageId(),
            _codec.fullName());
  auto full_name = _codec.fullName();
  auto pos = full_name.find("::");
  if (pos == std::string::npos) {
    return;
  }

  auto service_name = full_name.substr(0, pos);
  auto method_name = full_name.substr(pos + 2);

  auto service_it = _services->find(service_name);
  if (service_it == _services->end()) {
    LOG_ERROR("Service not found: {}", service_name);
    return;
  }
  auto service = service_it->second;
  if (service == nullptr) {
    LOG_ERROR("Service is nullptr: {}", service_name);
    return;
  }

  auto method = service->GetDescriptor()->FindMethodByName(method_name);
  if (method == nullptr) {
    LOG_ERROR("Method not found: {}", method_name);
    return;
  }
  auto request = service->GetRequestPrototype(method).New();
  auto succeeded = request->ParseFromString(_codec.body());
  if (!succeeded) {
    LOG_ERROR("Failed to parse request: {}", full_name);
    return;
  }

  auto response = service->GetResponsePrototype(method).New();
  auto done = google::protobuf::NewCallback(this, &Channel::onDoneRequest,
                                            _codec.messageId(), response);
  service->CallMethod(method, nullptr, request, response, done);
}

auto Channel::onResponseMessage() -> void {
  LOG_DEBUG("Response message id: {}", _codec.messageId());
  auto message_id = _codec.messageId();
  std::unique_lock lock(_mutex);
  auto sent_rpc_it = _sent_rpcs.find(message_id);
  if (sent_rpc_it == _sent_rpcs.end()) {
    LOG_ERROR("Sent rpc not found: {}", message_id);
    return;
  }
  lock.unlock();
  auto&& sent_rpc = sent_rpc_it->second;

  auto response = sent_rpc._responses;
  auto succeed = response->ParseFromString(_codec.body());
  if (!succeed) {
    LOG_ERROR("Failed to parse response: {}", message_id);
    return;
  }
  if (sent_rpc._closures != nullptr) {
    sent_rpc._closures->Run();
  }
  lock.lock();
  _sent_rpcs.erase(sent_rpc_it);
}

auto Channel::onDoneRequest(std::uint64_t message_id,
                            google::protobuf::Message* response) -> void {
  LOG_DEBUG("Done request. Message id: {}", message_id);
  auto buffer =
      Codec::makeMessageBuffer(message_id, false, "__::__", *response);
  _session.lock()->send(buffer);
}

}  // namespace fz::rpc

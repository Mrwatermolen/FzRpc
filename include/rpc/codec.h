#ifndef __FZ_RPC_CODEC_H__
#define __FZ_RPC_CODEC_H__

#include <google/protobuf/message.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "net/common/buffer.h"

namespace fz::rpc {

using Message = google::protobuf::Message;

class Codec {
 public:
  enum State : std::uint8_t {
    Invalid,
    MessageId,
    Flag,
    FullNameLength,
    FullName,
    BodyLength,
    Body,
    OK
  };

 public:
  static auto makeMessageBuffer(uint64_t id, bool request,
                                std::string full_name,
                                const google::protobuf::Message& message) {
    std::uint32_t full_name_length_length = full_name.size();
    auto body = message.SerializeAsString();
    std::uint32_t body_length = body.size();

    auto buffer = fz::net::Buffer{};
    buffer.append(reinterpret_cast<const char*>(&id), sizeof(id));
    buffer.append(reinterpret_cast<const char*>(&request), sizeof(request));
    buffer.append(reinterpret_cast<const char*>(&full_name_length_length),
                  sizeof(full_name_length_length));
    buffer.append(full_name.data(), full_name_length_length);
    buffer.append(reinterpret_cast<const char*>(&body_length),
                  sizeof(body_length));
    buffer.append(body.data(), body_length);
    return buffer;
  }

 public:
  auto state() const -> State { return _state; }

  auto messageId() const -> std::uint64_t { return _meta._message_id; }

  auto isRequest() const -> bool { return _meta._request; }

  auto fullName() const -> std::string { return _meta._full_name; }

  auto body() const -> std::string { return _meta._body; }

  auto read(fz::net::Buffer& buffer) -> void {
    while (_state != State::Invalid && buffer.readableBytes() != 0 &&
           _state != State::OK && parse(buffer)) {
    };
  }

  auto reset() {
    _state = State::MessageId;
    _meta = {};
  }

  auto markAsInvalid() { _state = State::Invalid; }

 private:
  auto parse(fz::net::Buffer& buffer) -> bool {
    switch (_state) {
      case MessageId: {
        if (buffer.readableBytes() < sizeof(std::uint64_t)) {
          return false;
        }

        auto message_id_data = buffer.readBegin();
        _meta._message_id =
            *reinterpret_cast<const std::uint64_t*>(message_id_data);
        buffer.retrieve(sizeof(std::uint64_t));

        _state = Flag;
        return true;
      }
      case Flag: {
        if (buffer.readableBytes() < sizeof(bool)) {
          return false;
        }

        auto flag_data = buffer.readBegin();
        _meta._request = *reinterpret_cast<const bool*>(flag_data);
        buffer.retrieve(sizeof(bool));

        _state = FullNameLength;
        return true;
      }
      case FullNameLength: {
        if (buffer.readableBytes() < sizeof(std::uint32_t)) {
          return false;
        }

        auto full_name_length_data = buffer.readBegin();
        _meta._full_name_length =
            *reinterpret_cast<const std::uint32_t*>(full_name_length_data);
        buffer.retrieve(sizeof(std::uint32_t));
        if (_meta._full_name_length < 3) {
          // invalid full name
          markAsInvalid();
          return false;
        }

        _state = FullName;
        return true;
      }
      case FullName: {
        if (buffer.readableBytes() < _meta._full_name_length) {
          return false;
        }

        auto full_name_data =
            std::string_view(buffer.readBegin(), _meta._full_name_length);

        assert(full_name_data.find("::") != std::string_view::npos);
        _meta._full_name = full_name_data;
        buffer.retrieve(_meta._full_name_length);

        _state = BodyLength;
        return true;
      }
      case BodyLength: {
        if (buffer.readableBytes() < sizeof(std::uint32_t)) {
          return false;
        }

        auto body_length_data = buffer.readBegin();
        _meta._body_length =
            *reinterpret_cast<const std::uint32_t*>(body_length_data);
        buffer.retrieve(sizeof(std::uint32_t));
        if (_meta._body_length == 0) {
          _state = OK;
          return true;
        }

        _state = Body;
        return true;
      }
      case Body: {
        const auto body_len = _meta._body_length;
        if (buffer.readableBytes() < body_len) {
          return false;
        }

        auto body_data = std::string_view(buffer.readBegin(), body_len);
        _meta._body = body_data;
        buffer.retrieve(body_len);
        _state = OK;
        return true;
      }
      case Invalid: {
        buffer.retrieve(buffer.readableBytes());
        return false;
      }
      default:
        return false;
    }

    return false;
  }

 private:
  State _state{State::MessageId};
  struct Meta {
    std::uint64_t _message_id;
    bool _request;
    std::uint32_t _full_name_length;
    std::string _full_name;
    std::uint32_t _body_length;
    std::string _body;
  };
  Meta _meta{};
};

}  // namespace fz::rpc

#endif  // __FZ_RPC_CODEC_H__

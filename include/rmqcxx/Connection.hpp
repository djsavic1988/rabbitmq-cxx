/*
Project: rabbitmq-cxx <https://github.com/djsavic1988/rabbitmq-cxx>

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT

Copyright (c) 2021 Djordje Savic <djordje.savic.1988@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#if __cplusplus < 201703L
#include <mpark/variant.hpp>
namespace std {

  template <typename... Ts>
  using variant = mpark::variant<Ts...>;
  using mpark::get;

} // namespace std
#else
#include <variant>
#endif

#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>

#include "Envelope.hpp"
#include "Exceptions.hpp"
#include "Message.hpp"
#include "ReturnedMessage.hpp"
#include "Timeout.hpp"
#include "util.hpp"

namespace rmqcxx {

class Channel;

/**
 * RMQ Connection class
 */
class Connection final {
public:
  /**
   * Consume Result type
   */
  using ConsumeResult = std::variant<Timeout, Envelope, ReturnedMessage, ::amqp_basic_ack_t>;

  /**
   * Enum to make access to the ConsumeResult easier to read
   *
   * @note Intentionally not an enum class for easier access
   */
  enum ConsumeResultIndex : std::size_t {
    TimeoutIdx = 0, /// When getting the envelope times out
    EnvelopeIdx, /// When the result contains an envelope
    ReturnedMessageIdx, /// When the result contains a returned (NACKed) message
    AcknowledgeIdx /// When the result contains acknowledge information (Publisher confirms)
  };

  /**
   * Constructs a connection
   *
   * @tparam ConnectionDuration Any std::chrono::duration compatible type
   * @tparam HandshakeDuration Any std::chrono::duration compatible type
   * @tparam Args TableEntry types
   *
   * @param[in] address Address of the RMQ broker
   * @param[in] port Port of the RMQ broker
   * @param[in] vhost VHost this connection will operate on
   * @param[in] maxChannels Maximum number of channels for this connnection
   * @param[in] maxFrameSize Maximum size of a single frame for this connection
   * @param[in] heartbeat Number of seconds between heartbeats to ask from the broker
   * @param[in] connectTimeout Maximum duration for trying to connect
   * @param[in] handshakeTimeout Maximum duration for trying to do a handshake
   * @param[in] properties Connection properties
   * @param[in] saslMethod AMQP SASL method
   * @param[in] args Login arguments
   *
   * @throw ChannelCloseException When channel for the login RPC should be closed - This should never happen as there is no channel as the point of constructing this
   * @throw ConnectionCloseException When connection for the login RPC should be closed
   * @throw Exception When allocating the connection fails
   * @throw LibraryException When there is a library exception
   * @throw OperationException When setting the handshake timeout fails
   * @throw RPCException For general RPC exception on login
   * @throw SocketException When socket can't be allocated and when socket opening fails
   *
   */
  template <typename ConnectionDuration, typename HandshakeDuration, typename... Args>
  Connection(
    const std::string& address, int port, const std::string& vhost, int maxChannels, int maxFrameSize, int heartbeat,
    ConnectionDuration connectTimeout, const HandshakeDuration* handshakeTimeout, const amqp_table_t *properties, ::amqp_sasl_method_enum saslMethod,
    Args... args) : connection_(::amqp_new_connection(), ::amqp_destroy_connection), context_(std::string("Connection(") + std::to_string(reinterpret_cast<uint64_t>(connection_.get())) + "): ") {

    if (!connection_) {
      throw Exception("Failed to allocate connection object!");
    }
    auto socket = ::amqp_tcp_socket_new(connection_.get());
    if (nullptr == socket) {
      throw SocketException(*this, socket, AMQP_STATUS_SOCKET_ERROR, "Failed to allocate socket object!");
    }

    auto tv = timeValue(connectTimeout);
    auto socketStatus = static_cast<::amqp_status_enum>(::amqp_socket_open_noblock(socket, address.c_str(), port, &tv));
    if (AMQP_STATUS_OK != socketStatus) {
      close();
      throw SocketException(*this, socket, socketStatus, "Failed to open socket!");
    }

    if (handshakeTimeout != nullptr) {
      auto hsTv = timeValue(*handshakeTimeout);
      auto status = ::amqp_set_handshake_timeout(connection_.get(), &hsTv);
      if (AMQP_STATUS_OK != status) {
        close();
        throw OperationException(*this, status, "Failed to set handshake timeout!");
      }
    }

    auto reply = nullptr == properties ?
    ::amqp_login(connection_.get(), vhost.c_str(), maxChannels, maxFrameSize, heartbeat, saslMethod, std::forward<Args>(args)...) :
    ::amqp_login_with_properties(connection_.get(), vhost.c_str(), maxChannels, maxFrameSize, heartbeat, properties, saslMethod, std::forward<Args>(args)...);
    try {
      processReply(context_, reply);
    } catch(...) {
      close();
      throw;
    }
  }

  /**
   * Connection constructor that uses plain SASL
   *
   * @tparam Duration std::chrono::duration compatible duration type
   *
   * @param[in] address Address of the RMQ broker
   * @param[in] port Port of the RMQ broker
   * @param[in] username Username
   * @param[in] password Password
   * @param[in] vhost VHost
   * @param[in] maxChannels Maximum number of channels for this connnection
   * @param[in] maxFrameSize Maximum size of a single frame for this connection
   * @param[in] heartbeat Number of seconds between heartbeats to ask from the broker
   * @param[in] timeout Connect timeout
   *
   * @throw ChannelCloseException When channel for the login RPC should be closed - This should never happen as there is no channel as the point of constructing this
   * @throw ConnectionCloseException When connection for the login RPC should be closed
   * @throw Exception When allocating the connection fails
   * @throw LibraryException When there is a library exception
   * @throw OperationException When setting the handshake timeout fails
   * @throw RPCException For general RPC exception on login
   * @throw SocketException When socket can't be allocated and when socket opening fails
   */
  template <typename Duration>
  Connection(
    const std::string& address, int port, const std::string& username, const std::string& password, const std::string& vhost,
    int maxChannels, int maxFrameSize, int heartbeat, Duration timeout) :
      Connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, timeout, static_cast<const std::chrono::seconds*>(nullptr), nullptr, AMQP_SASL_METHOD_PLAIN, username.c_str(), password.c_str()) {}

  /**
   * Connection constructor that uses external SASL method
   *
   * @tparam Duration std::chrono::duration compatible duration type
   *
   * @param[in] address Address of the RMQ broker
   * @param[in] port Port of the RMQ broker
   * @param[in] info Info string
   * @param[in] vhost VHost
   * @param[in] maxChannels Maximum number of channels for this connnection
   * @param[in] maxFrameSize Maximum size of a single frame for this connection
   * @param[in] heartbeat Number of seconds between heartbeats to ask from the broker
   * @param[in] timeout Connect timeout
   *
   * @throw ChannelCloseException When channel for the login RPC should be closed - This should never happen as there is no channel as the point of constructing this
   * @throw ConnectionCloseException When connection for the login RPC should be closed
   * @throw Exception When allocating the connection fails
   * @throw LibraryException When there is a library exception
   * @throw OperationException When setting the handshake timeout fails
   * @throw RPCException For general RPC exception on login
   * @throw SocketException When socket can't be allocated and when socket opening fails
   */
  template <typename Duration>
  Connection(
    const std::string& address, int port, const std::string& info, const std::string& vhost, int maxChannels, int maxFrameSize,
    int heartbeat, Duration timeout) :
      Connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, timeout, static_cast<const std::chrono::seconds*>(nullptr), nullptr, AMQP_SASL_METHOD_EXTERNAL, info.c_str()) {}

  /**
   * Destructor
   */
  ~Connection() noexcept {
    close();
  }

  /**
   * Can't be copy constructed
   */
  Connection(const Connection&) = delete;

  /**
   * Move constructable
   */
  Connection(Connection&&) = default;

  /**
   * Can't be copy assigned
   */
  Connection& operator=(const Connection&) = delete;

  /**
   * Can't be move assigned
   */
  Connection& operator=(Connection&&) noexcept = delete;

  /**
   * Does a RPC on this connection.
   *
   * @tparam Function RPC method to call
   * @tparam Args Arguments of the RPC method
   *
   * @param[in] f Method to execute on the broker
   * @param[in] args Arguments for the method
   *
   * @return Whatever the remote method returns
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename Function, typename... Args>
  auto rpc(const Function& f, Args&&... args) -> decltype(f(::amqp_connection_state_t(), std::forward<Args>(args)...)) {
    return rpc("", f, std::forward<Args>(args)...);
  }

  /**
   * Consumes broker messages
   *
   * @tparam Duration std::chrono::duration compatible type
   *
   * @param[in] timeout Duration after which this client times out
   *
   * @return Result of consuming depends on the broker state, one value of the varient will be set
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw FrameException When a frame exception happens
   * @throw FrameStatusException When an exception occurs while waiting for a frame
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   * @throw SocketException On socket error
   */
  template <typename Duration>
  ConsumeResult consume(Duration timeout) {
    auto tv = timeValue(timeout);
    return consumeImpl(&tv);
  }

  /**
   * Consumes broker messages by blocking until there is an error or a message
   *
   * @param[in] timeout Duration after which this client times out
   *
   * @return Result of consuming depends on the broker state, one value of the varient will be set
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw FrameException When a frame exception happens
   * @throw FrameStatusException When an exception occurs while waiting for a frame
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   * @throw SocketException On socket error
   */
  ConsumeResult consume() {
    return consumeImpl(nullptr);
  }

  /**
   * Gets the RPC timeout from the underlying library
   *
   * @tparam Duration std::chrono::duration compatible type
   *
   * @param[out] dst Duration of the RPC timeout
   *
   * @return True on success
   */
  template <typename Duration>
  bool getRpcTimeout(Duration& dst) const noexcept {
    const auto p = ::amqp_get_rpc_timeout(connection_.get());
    if (nullptr == p)
      return false;
    dst = std::chrono::duration_cast<Duration>(std::chrono::seconds(p->tv_sec))
      + std::chrono::duration_cast<Duration>(std::chrono::microseconds(p->tv_usec));
    return true;
  }

  /**
   * Sets the RPC timeout for the underlying library
   *
   * @tparam Duration std::chrono::duration compatible type
   *
   * @param[in] val Duration to set as the RPC timeout
   */
  template <typename Duration>
  void setRpcTimeout(Duration val) const {
    auto tv = timeValue(val);
    if (AMQP_STATUS_OK != ::amqp_set_rpc_timeout(connection_.get(), &tv))
      throw ConnectionException(*this, "Failed to set RPC timeout!");
  }

  /**
   * Conversion to the raw connection pointer
   */
  inline operator ::amqp_connection_state_t() const noexcept {
    return connection_.get();
  }

private:

  /**
   * Closes the connection if possible
   */
  void close() noexcept {
    if (!connection_)
      return;
    try {
      rpc(::amqp_connection_close, AMQP_REPLY_SUCCESS); // gracefully close
    } catch(...) {

    }
  }

  /**
   * Does a RPC on this connection.
   *
   * @tparam Function Type of the RPC method to call
   * @tparam Args Types of arguments of the RPC method
   *
   * @param[in] context String describing the context of the RPC
   * @param[in] f Method to execute on the broker
   * @param[in] args Arguments for the method
   *
   * @return Whatever the remote method returns
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename Function, typename... Args>
  auto rpc(const std::string& context, const Function& f, Args&&... args) -> decltype(f(::amqp_connection_state_t(), std::forward<Args>(args)...)) {
    const auto& c = connection_.get();
    defer g{ [this, c, context] () {
      processReply(context_ + context, ::amqp_get_rpc_reply(c));
    }};
    return f(c, std::forward<Args>(args)...);
  }

  /**
   * Processes an RPC reply
   *
   * @param[in] context Context used for Exceptions
   * @param[in] reply RPC reply
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  void processReply(const std::string& context, const ::amqp_rpc_reply_t& reply) const {
    switch (reply.reply_type) {
      case AMQP_RESPONSE_NORMAL:
        return;
      case AMQP_RESPONSE_NONE:
        throw RPCException(*this, reply, context + "Missing RPC Reply!");
      case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        throw LibraryException(*this, reply, context + "Library exception: ");
      case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch(reply.reply.id) {
          case AMQP_CONNECTION_CLOSE_METHOD:
            throw ConnectionCloseException(*this, static_cast<const ::amqp_connection_close_t*>(reply.reply.decoded), context + "Connection close received. ");
          case AMQP_CHANNEL_CLOSE_METHOD:
            throw ChannelCloseException(*this, 0, static_cast<const ::amqp_channel_close_t*>(reply.reply.decoded), context + "Channel close received. ");
          default:
            throw RPCException(*this, reply, context + "Unhandled Server Response Exception, method name: " + ::amqp_method_name(reply.reply.id));
        }
      default:
        throw RPCException(*this, reply, context + "Unhandled reply type!");
    }
  }

  /**
   * AMQP Consume implementation
   *
   * @param[in] tv Timeout, set to nullptr to block until there is a message or an error
   *
   * @return ConsumeResult
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw FrameException When a frame exception happens
   * @throw FrameStatusException When an exception occurs while waiting for a frame
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   * @throw SocketException On socket error
   *
   * @note Timeout could be upto double of that set because it is reused as a parameter to wait for a frame in case of AMQP_STATUS_UNEXPECTED_STATE
   */
  ConsumeResult consumeImpl(timeval* tv) {
    Envelope envelope;
    auto reply = ::amqp_consume_message(connection_.get(), static_cast<::amqp_envelope_t*>(envelope), tv, 0 /*Always 0, requested by the library*/);
    switch(reply.reply_type) {
      case AMQP_RESPONSE_NORMAL:
        if (envelope->channel == 0)
          return Timeout();
        return envelope;
      case AMQP_RESPONSE_LIBRARY_EXCEPTION: {
        switch(reply.library_error) {
          case AMQP_STATUS_UNEXPECTED_STATE: {
            ::amqp_frame_t frame;
            auto status = ::amqp_simple_wait_frame_noblock(connection_.get(), &frame, tv);
            switch(status) {
              case AMQP_STATUS_OK:
                break;
              case AMQP_STATUS_TIMEOUT:
                return Timeout();
              default:
                throw FrameStatusException(*this, reply, status, context_ + "Consumer: Received unhandled status when waiting for frame");
            }
            if (AMQP_FRAME_METHOD != frame.frame_type)
              throw FrameException(*this, reply, frame, context_ + "Consumer: Received unhandled frame type!"); // getting the frame failed but we don't know what to do

            switch(frame.payload.method.id) {
              case AMQP_BASIC_ACK_METHOD:
                return *static_cast<const ::amqp_basic_ack_t*>(frame.payload.method.decoded);
              case AMQP_BASIC_RETURN_METHOD: {
                Message message;
                processReply(context_ + " Consumer (return method): ", ::amqp_read_message(connection_.get(), frame.channel, static_cast<::amqp_message_t*>(message), 0));
                return ReturnedMessage(std::move(message), *static_cast<const ::amqp_basic_return_t*>(frame.payload.method.decoded));
              }
              case AMQP_CHANNEL_CLOSE_METHOD:
                throw ChannelCloseException(*this, frame.channel, static_cast<const ::amqp_channel_close_t*>(frame.payload.method.decoded), context_ + "Consumer: Channel close received!");

              case AMQP_CONNECTION_CLOSE_METHOD:
                throw ConnectionCloseException(*this, static_cast<const ::amqp_connection_close_t*>(frame.payload.method.decoded), context_ + "Consumer: Connection close received!");
              default:
                throw FrameException(*this, reply, frame, context_ + "Consumer: Received unhandled method: " + ::amqp_method_name(frame.payload.method.id));
            }
          }
            break;
          case AMQP_STATUS_TIMEOUT:
            return Timeout();

          case AMQP_STATUS_SOCKET_ERROR:
            throw SocketException(*this, nullptr, AMQP_STATUS_OK, context_ + "Consumer: Socket Error!");

          default:
            throw RPCException(*this, reply, context_ + "Consumer: Received an unhandled library exception!");
        }
      }
      default:
        throw RPCException(*this, reply, context_ + "Consumer: Received an unhandled RPC reply!");
    }
  }

  /**
   * Connection storage
   */
  std::unique_ptr<std::remove_pointer<decltype(::amqp_new_connection())>::type, decltype(&::amqp_destroy_connection)> connection_;

  /**
   * Context used for logging, exceptions and errors
   */
  const std::string context_;

  friend class Channel;
};

} // namespace rmqcxx

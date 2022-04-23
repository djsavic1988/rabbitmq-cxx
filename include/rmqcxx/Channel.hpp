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

#include "Connection.hpp"
#include "Table.hpp"

namespace rmqcxx {

/**
 * Wraps ::amqp_channel_t and method calls at the channel level
 */
class Channel final {
public:
  /**
   * Constructs a channel
   *
   * @param[in] connection Reference to a Connection object using which this channel will operate on
   * @param[in] channel Id of the channel
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  Channel(Connection& connection, ::amqp_channel_t channel) :
    connection_(connection),
    channel_(channel),
    context_(std::string("Channel(") + std::to_string(channel) + "): "),
    moved_(false) {
    connection_.rpc(true, this->context_, ::amqp_channel_open, channel_);
  }

  /**
   * Destructs this channel gracefully
   */
  ~Channel() noexcept {
      if (moved_)
        return;
      try {
        connection_.rpc(::amqp_channel_close, channel_, AMQP_REPLY_SUCCESS);
      }
      catch(...) {
        // don't care if this fails
      }
  }

  /**
   * Can't be copy constructed
   */
  Channel(const Channel&) = delete;

  /**
   * Move constructable
   */
  Channel(Channel&& other) : connection_(other.connection_), channel_(other.channel_), context_(other.context_), moved_(false) {
    other.moved_ = true;
  }

  /**
   * Can't be copy assigned
   */
  Channel& operator=(const Channel&) = delete;

  /**
   * Can't be move assigned
   */
  Channel& operator==(Channel&&) noexcept = delete;

  /**
   * Acknowledges messages
   *
   * @param[in] tag Message delivery tag
   * @param[in] multiple If set acknowledges all messages that were received prior to the passed tag
   *
   * @return 0 on success, everything else should be considered a failure
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  int ack(uint64_t tag, bool multiple) {
    return rpc(::amqp_basic_ack, tag, multiple);
  }

  /**
   * Negative acknowledge (return) for message(s)
   *
   * @param[in] tag Tag of a message
   * @param[in] multiple If set nacks all messages that were received prior to the passed tag
   * @param[in] requeue If set will try to return messages to their queue, otherwise deadletters messages
   *
   * @return AMQP_STATUS_OK on success, an amqp_status_enum value otherwise.
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  int nack(uint64_t tag, bool multiple, bool requeue) {
    return rpc(::amqp_basic_nack, tag, multiple, requeue);
  }

  /**
   * Performs an RPC on the connection using this channel
   *
   * @tparam Function AMQP remote procedure type
   * @tparam Args AMQP remote procedure arguments types
   *
   * @param[in] f Method to execute (ie: ::amqp_basic_ack)
   * @param[in] args Arguments to pass to the desired method
   *
   * @return Whatever the remote procedure returns
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename Function, typename... Args>
  auto rpc(const Function& f, Args&&... args) -> decltype(f(::amqp_connection_state_t(), ::amqp_channel_t(), std::forward<Args>(args)...)) {
   return rpc(this->context_, f, std::forward<Args>(args)...);
  }

  /**
   * QoS method
   *
   * @param[in] prefetchCount Prefetch count
   * @param[in] perChannel If set will instruct the broker to set the desired options per channel
   * @param[in] prefetchSize Prefetch size
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  void qos(uint16_t prefetchCount, bool perChannel = false, uint32_t prefetchSize = 0) {
    rpc(::amqp_basic_qos, prefetchSize, prefetchCount, perChannel);
  }

  /**
   * Sets flow from peer
   *
   * @param[in] active If set peer stats sending content frames
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  bool flow(bool active) {
    return rpc(::amqp_channel_flow, active)->active;
  }

  /**
   * Redeliver unacknowledged messages
   *
   * @param[in] requeue If set to false the message will be delivered to the original recipient, otherwise the server will requeue the message
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  void recover(bool requeue) {
    rpc(::amqp_basic_recover, requeue);
  }

  /**
   * Publishes a message on this channel
   *
   * @param[in] exchange Exchange name
   * @param[in] routingKey Routing key
   * @param[in] mandatory If set to true then if the message can't be routed the connection will receive basic return method
   * @param[in] immediate If set to true then if the message can't be immediately consumed the connection will receive basic return method
   * @param[in] body Content to publish
   * @param[in] properties Any extra properties for publishing
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ChannelException When publishing fails
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  void publish(const std::string& exchange, const std::string& routingKey, bool mandatory, bool immediate, const std::string& body, const ::amqp_basic_properties_t& properties = amqp_basic_properties_t {0}) {
    switch(rpc(::amqp_basic_publish, bytes(exchange), bytes(routingKey),
      mandatory, immediate, &properties, bytes(body))) {
      case AMQP_STATUS_OK:
        break;
      default:
        throw ChannelException(connection_, *this,
          this->context_ + "Failed to publish message to exchange: " + exchange
          + " with routingKey: " + routingKey
          + " mandatory: " + (mandatory ? "true" : "false")
          + " immediate: " + (immediate ? "true" : "false")
          + " properties: " + "<not serialized>"
          + " body: " + body
        );
    }
  }

private:

  /**
   * Performs an RPC on the connection using this channel
   *
   * @tparam Function AMQP remote procedure type
   * @tparam Args AMQP remote procedure arguments types
   *
   * @param[in] context Context description
   * @param[in] f Method to execute (ie: ::amqp_basic_ack)
   * @param[in] args Arguments to pass to the desired method
   *
   * @return Whatever the remote procedure returns
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename Function, typename... Args>
  auto rpc(const std::string& context, const Function& f, Args&&... args) -> decltype(f(::amqp_connection_state_t(), ::amqp_channel_t(), std::forward<Args>(args)...)) {
    defer g([this] () {
      ::amqp_maybe_release_buffers_on_channel(static_cast<amqp_connection_state_t>(connection_), channel_);
    });
    return connection_.rpc(false, this->context_ + context, f, channel_, std::forward<Args>(args)...);
  }

  /**
   * Reference to the connection
   */
  Connection& connection_;

  /**
   * Channel identifier
   */
  ::amqp_channel_t channel_;

  /**
   * Context of this channel (used for exceptions, errors and logging)
   */
  const std::string context_;

  /**
   * Flag that tells this object if it was moved
   */
  bool moved_;

  friend class Exchange;
  friend class Queue;
};

} // namespace rmqcxx

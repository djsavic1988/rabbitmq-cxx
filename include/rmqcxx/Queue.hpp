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

#include <vector>

#include "Channel.hpp"
#include "Table.hpp"
#include "util.hpp"

namespace rmqcxx {

/**
 * Wrapper around an AMQP queue
 */
class Queue final {
public:
  /**
   * Constructs a queue
   *
   * @note No queue will be constructed on the broker after this call
   */
  Queue(Channel& channel, std::string name) noexcept :
    channel_(channel), name_(std::move(name)), context_(std::string("Queue(") + name_ + "): ") {}

  /**
   * Destructor
    */
  ~Queue() noexcept = default;

  /**
   * Can't be copy constructed
   */
  Queue(const Queue&) = delete;

  /**
   * Move constructable
   */
  Queue(Queue&&) = default;

  /**
   * Can't be copy assigned
   */
  Queue& operator=(const Queue&) = delete;

  /**
   * Can't be move assigned
   */
  Queue& operator=(Queue&&) noexcept = delete;

  /**
   * Does an RPC on the associated channel with the associated queue name
   *
   * @tparam Function RPC method type
   * @tparam Args RPC method parameters type
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename Function, typename... Args>
  auto rpc(const Function& f, Args&&... args)
    -> decltype(f(::amqp_connection_state_t(), ::amqp_channel_t(), ::amqp_bytes_t(), std::forward<Args>(args)...)) {
    return channel_.rpc(this->context_, f, bytes(name_), std::forward<Args>(args)...);
  }

  /**
   * Declares the queue on the broker
   *
   * @tparam Args TableEntry types
   *
   * @param[in] passive If set only checks the queue with the same name exists (no queue is created)
   * @param[in] durable If set the queue will persist after broker restart
   * @param[in] exclusive If set the declared queue will be exclusive for the connection it was created on
   * @param[in] autoDelete If set queue will be deleted when there are no more consumers
   * @param[in] args Any extra parameters associated with this queue
   *
   * @return Result describing the created queue
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  const amqp_queue_declare_ok_t* declare(bool passive, bool durable, bool exclusive, bool autoDelete, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    return rpc(::amqp_queue_declare, passive, durable, exclusive, autoDelete, static_cast<::amqp_table_t>(arguments));
  }

  /**
   * Binds this queue
   *
   * @tparam Args TableEntry types
   *
   * @param[in] exchange Name of the exchange to bind this queue to
   * @param[in] routingKey Routing key to bind this queue with
   * @param[in] args Any extra parameters associated with this queue binding
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  void bind(const std::string& exchange, const std::string& routingKey, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    rpc(::amqp_queue_bind, bytes(exchange), bytes(routingKey), arguments);
  }

  /**
   * Unbinds this queue
   *
   * @tparam Args TableEntry types
   *
   * @param[in] exchange Name of the exchange to bind this queue to
   * @param[in] routingKey Routing key to bind this queue with
   * @param[in] args Any extra parameters associated with this queue unbinding
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  void unbind(const std::string& exchange, const std::string& routingKey, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    rpc(::amqp_queue_unbind, bytes(exchange), bytes(routingKey), arguments);
  }


  /**
   * Registers to start consuming from this queue
   *
   * @tparam Args TableEntry types
   *
   * @param[in] consumerTag Consumer specific tag (one will be assigned if empty, must be unique for a channel)
   * @param[in] noLocal If this is set then the server will not send messages to the connection that published them
   * @param[in] noAck If this is set then the messages require no acknowledge (auto ack)
   * @param[in] exclusive If this is set then exclusive access for consuming will be requested from the broker
   * @param[in] args Any extra properties for consuming
   *
   * @return Consumer tag
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  std::string consume(const std::string& consumerTag, bool noLocal, bool noAck, bool exclusive, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    return container<std::string>(rpc(::amqp_basic_consume, bytes(consumerTag), noLocal, noAck, exclusive, arguments)->consumer_tag);
  }

  /**
   * Deletes this queue from the broker
   *
   * @param[in] ifUnused Only delete the queue if there are no consumers
   * @param[in] ifEmpty Only delete the queue if it's empty
   *
   * @return Number of deleted messages with the queue
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  uint32_t remove(bool ifUnused, bool ifEmpty) {
    return rpc(::amqp_queue_delete, ifUnused, ifEmpty)->message_count;
  }

  /**
   * Purges this queue
   *
   * @return Number of purged messages
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  uint32_t purge() {
    return rpc(::amqp_queue_purge)->message_count;
  }

  /**
   * Queue name access
   * @return Const reference to this queue name
   */
  const std::string& name() const {
    return name_;
  }

private:

  /**
   * Reference to a channel to perform RPCs on
   */
  Channel& channel_;

  /**
   * Name of the queue
   */
  std::string name_;

  /**
   * Context used for logging
   */
  const std::string context_;
};
} // namespace rmqcxx

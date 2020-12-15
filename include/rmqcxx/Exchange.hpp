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

#include "Channel.hpp"

namespace rmqcxx {

/**
 * Class that wraps AMQP Exchange
 */
class Exchange final {
public:

  /**
   * Constructs an Exchange description on a Channel
   *
   * @param[in] channel Reference to a channel that the excange operations will be performed on
   * @param[in] name Name of the exchange
   *
   * @note No exchange is actually declared at this point, rather just an exchange description in memory is
   * created
   */
  Exchange(Channel& channel, std::string name) noexcept :
    channel_(channel), name_(std::move(name)), context_(std::string("Exchange(") + name_ + "): "), moved_(false) {}

  /**
   * Destructor
   */
  ~Exchange() noexcept = default;

  /**
   * Can't be copy constructed
   */
  Exchange(const Exchange&) = delete;

  /**
   * Move constructable
   */
  Exchange(Exchange&&) = default;

  /**
   * Can't be copy assigned
   */
  Exchange& operator=(const Exchange&) = delete;

  /**
   * Can't be move assigned
   */
  Exchange& operator=(Exchange&&) noexcept = delete;

  /**
   * Does an RPC on the associated channel with the associated exchange name
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
  auto rpc(const Function& f, Args&&... args) noexcept -> decltype(f(::amqp_connection_state_t(), ::amqp_channel_t(), ::amqp_bytes_t(), std::forward<Args>(args)...)) {
    return channel_.rpc(this->context_, f, bytes(name_), std::forward<Args>(args)...);
  }

  /**
   * Declares an exchange on the broker
   *
   * @tparam Args TableEntry types
   *
   * @param[in] type Type of the exchange
   * @param[in] passive Does not create an excange will check if an exchange with the same name exists
   * @param[in] durable If set to true will persist after server reboots
   * @param[in] autoDelete If set tells the server to delete the exchange after all queues are done using it
   * @param[in] args Extra paramters associated with this exchange
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  void declare(const std::string& type, bool passive, bool durable, bool autoDelete, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    rpc(::amqp_exchange_declare, bytes(type), passive, durable, autoDelete, 0, static_cast<::amqp_table_t>(arguments));
  }

  /**
   * Binds this exchange to another exchange
   *
   * @tparam Args TableEntry types
   * @param[in] src Source exchange name
   * @param[in] routingKey Routing key to bind with
   * @param[in] args Any extra parameters for this binding
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  void bind(const std::string& src, const std::string& routingKey, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    rpc(::amqp_exchange_bind, bytes(src), bytes(routingKey), static_cast<::amqp_table_t>(arguments));
  }

  /**
   * Unbinds this exchange from another exchange
   *
   * @tparam Args TableEntry types
   *
   * @param[in] src Source exchange name
   * @param[in] routingKey Routing key to unbind from
   * @param[in] args Any extra parameters for this unbinding
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  template <typename... Args>
  void unbind(const std::string& src, const std::string& routingKey, Args&&... args) {
    Table arguments(std::forward<Args>(args)...);
    rpc(::amqp_exchange_unbind, bytes(src), bytes(routingKey), static_cast<::amqp_table_t>(arguments));
  }

  /**
   * Deletes this exchange from the broker
   *
   * @param[in] ifUnused Will only delete the exchange if it's unused
   *
   * @throw ChannelCloseException When channel for the executed RPC should be closed
   * @throw ConnectionCloseException When connection for the executed RPC should be closed
   * @throw LibraryException When there is a library exception
   * @throw RPCException For general RPC exception
   */
  void remove(bool ifUnused) {
    rpc(::amqp_exchange_delete, ifUnused);
  }

  /**
   * Name of the exchange
   * @return Name of this exchange
   */
  const std::string& name() const {
    return name_;
  }

private:

  /**
   * Reference to the channel that will be used for RPC
   */
  Channel& channel_;

  /**
   * Name of the exchange
   */
  std::string name_;

  /**
   * Context used for logging
   */
  const std::string context_;

  /**
   * Flag to tell us if this object has been moved
   */
  bool moved_;
};
} // namespace rmqcxx

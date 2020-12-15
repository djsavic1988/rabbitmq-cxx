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

#include <stdexcept>

#include <amqp.h>

#include "util.hpp"

namespace rmqcxx {

  class Channel;
  class Connection;

  /**
   * Base exception type used by this library
   */
  struct Exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  /**
   * Execption that relates to the connection
   */
  struct ConnectionException : public Exception {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] reason Reason for throwing this exception
     */
    ConnectionException(const Connection& connection, std::string reason) noexcept : Exception(std::move(reason)), connection(connection) {}

    /**
     * Reference to the connection object
     */
    const Connection& connection;
  };

  /**
   * Socket exception
   */
  struct SocketException : public ConnectionException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] socket Pointer to the socket that is associated with this exception
     * @param[in] status Socket status
     * @param[in] reason Reason for throwing this exception
     */
    SocketException(const Connection& connection, const ::amqp_socket_t* socket, ::amqp_status_enum status, std::string reason) noexcept :
      ConnectionException(connection, std::move(reason)), socket(socket), status(status) {}

      /**
       * Socket pointer (non owning)
       */
      const ::amqp_socket_t* socket;

      /**
       * Socket status
       */
      const ::amqp_status_enum status;
  };

  /**
   * Exception that relates to an attempted operation on the connection
   */
  struct OperationException : public ConnectionException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] status Operation status
     * @param[in] reason Reason for throwing this exception
     */
    OperationException(const Connection& connection, int status, std::string reason) noexcept : ConnectionException(connection, std::move(reason)), status(status) {}

    /**
     * Operation status
     */
    const int status;
  };

  /**
   * Exception while executing an RPC
   */
  struct RPCException : public ConnectionException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] reply RPC reply object
     * @param[in] reason Reason for throwing this exception
     */
    RPCException(const Connection& connection, ::amqp_rpc_reply_t reply, std::string reason) noexcept : ConnectionException(connection, std::move(reason)), reply(reply) {}

    /**
     * RPC reply object storage
     */
    const ::amqp_rpc_reply_t reply;
  };

  /**
   * Exception that relates to AMQP frame status
   */
  struct FrameStatusException : public RPCException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] reply RPC reply object
     * @param[in] status Frame status
     * @param[in] reason Reason for throwing this exception
     */
    FrameStatusException(const Connection& connection, ::amqp_rpc_reply_t reply, int status, std::string reason) noexcept :
      RPCException(connection, reply, std::move(reason)), status(status) {}

      /**
       * Frame status
       */
      const int status;
  };

  /**
   * Exception that relates to an AMQP frame
   */
  struct FrameException : public RPCException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] reply RPC reply object
     * @param[in] frame Frame
     * @param[in] reason Reason for throwing this exception
     */
    FrameException(const Connection& connection, ::amqp_rpc_reply_t reply, ::amqp_frame_t frame, std::string reason) noexcept :
      RPCException(connection, reply, std::move(reason)), frame(frame) {}

      /**
       * Frame storage
       */
      const ::amqp_frame_t frame;
  };

  /**
   * Channel closed received from the broker
   */
  struct ChannelCloseException : public ConnectionException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] channel Channel identifier
     * @param[in] decoded Channel close method pointer
     * @param[in] reason Reason for throwing this exception
     */
    ChannelCloseException(const Connection& connection, ::amqp_channel_t channel, const ::amqp_channel_close_t* decoded, std::string reason) noexcept :
      ConnectionException(connection, std::move(reason) + ' ' + decodeAmqpMethod(decoded)), channel(channel) {}

    /**
      * Channel id storage
     */
    ::amqp_channel_t channel;
  };

  /**
   * Connection closed received from the broker
   */
  struct ConnectionCloseException : public ConnectionException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] decoded Connection close method pointer
     * @param[in] reason Reason for throwing this exception
     */
    ConnectionCloseException(const Connection& connection, const ::amqp_connection_close_t* decoded, std::string reason) :
      ConnectionException(connection, std::move(reason) + ' ' + decodeAmqpMethod(decoded)) {}
  };

  /**
   * AMQP Library exception
   */
  struct LibraryException : public RPCException {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] reply Reply received from the underlying library
     * @param[in] reason Reason for throwing this exception
     */
    LibraryException(const Connection& connection, ::amqp_rpc_reply_t reply, std::string reason) noexcept :
      RPCException(connection, reply, std::move(reason) + ::amqp_error_string2(reply.library_error)) {}
  };

  /**
   * Channel based exception
   */
  struct ChannelException : public Exception {
    /**
     * Constructor
     *
     * @param[in] connection Reference to the connection object
     * @param[in] channel Channel identifier
     * @param[in] reason Reason for throwing this exception
     */
    ChannelException(const Connection& connection, const Channel& channel, std::string reason) noexcept : Exception(std::move(reason)), connection(connection), channel(channel) {}

    /**
     * Reference to the connection object
     */
    const Connection& connection;

    /**
     * Reference to the channel object
     */
    const Channel& channel;
  };

} // namespace rmqcxx

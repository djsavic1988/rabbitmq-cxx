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
#include <string>
#include <vector>

#include <amqp.h>

#include <gtest/gtest.h>

#include <rmqcxx/Connection.hpp>

#include "comparison.hpp"
#include "MockAMQP.hpp"

namespace rmqcxx { namespace unit_tests {
struct ConnectionTest : public ::testing::Test {
  ConnectionTest() : address("address"), port(1), vhost("vhost"), maxChannels(2), maxFrameSize(3), heartbeat(4), connectTimeout(5), handshakeTimeout(6), properties(), saslMethod(AMQP_SASL_METHOD_UNDEFINED), connPtr(reinterpret_cast<amqp_connection_state_t>(7)), socketPtr(reinterpret_cast<amqp_socket_t*>(8)), connectTv{.tv_sec = connectTimeout.count(), .tv_usec = 0}, handshakeTv{.tv_sec = handshakeTimeout.count(), .tv_usec = 0}, normalReply{.reply_type = AMQP_RESPONSE_NORMAL } {

  }

  template <typename... Args>
  void prepareConnectionCreation(bool withProperties, bool withHandshakeTimeout, Args... loginArgs) {
    std::vector<const char*> loginArguments {{std::forward<Args>(loginArgs)...}};
    amqp_rpc_reply_t reply{.reply_type = AMQP_RESPONSE_NORMAL};

    EXPECT_CALL(amqp, new_connection())
      .WillOnce(::testing::Return(connPtr));

    EXPECT_CALL(amqp, tcp_socket_new(connPtr))
      .WillOnce(::testing::Return(socketPtr));

    EXPECT_CALL(amqp, socket_open_noblock(socketPtr, address.c_str(), port, ::testing::Pointee(connectTv)))
      .WillOnce(::testing::Return(0));

    if (withHandshakeTimeout) {
      EXPECT_CALL(amqp, set_handshake_timeout(connPtr, ::testing::Pointee(handshakeTv)))
        .WillOnce(::testing::Return(AMQP_STATUS_OK));
    }

    if (withProperties) {
      EXPECT_CALL(amqp, login_with_properties(connPtr, vhost.c_str(), maxChannels, maxFrameSize, heartbeat, &properties, saslMethod, loginArguments))
        .WillOnce(::testing::Return(reply));
    } else {
      EXPECT_CALL(amqp, login(connPtr, vhost.c_str(), maxChannels, maxFrameSize, heartbeat, saslMethod, loginArguments))
        .WillOnce(::testing::Return(reply));
    }

    EXPECT_CALL(amqp, connection_close(connPtr, AMQP_REPLY_SUCCESS));

    EXPECT_CALL(amqp, get_rpc_reply(connPtr, "connection_close"))
      .WillOnce(::testing::Return(amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_NORMAL }));

    EXPECT_CALL(amqp, destroy_connection(connPtr));
  }

  template <typename... Args>
  Connection createConnection(bool withProperties, bool withHandshakeTimeout, Args... loginArgs) {
    prepareConnectionCreation(withProperties, withHandshakeTimeout, std::forward<Args>(loginArgs)...);
    return Connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, withHandshakeTimeout ? &handshakeTimeout : static_cast<const std::chrono::seconds*>(nullptr), withProperties ? &properties : nullptr, saslMethod, std::forward<Args>(loginArgs)...);
  }

  Connection createSimpleConnection() {
    saslMethod = AMQP_SASL_METHOD_EXTERNAL;
    prepareConnectionCreation(false, false, "external");
    return Connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, static_cast<const std::chrono::seconds*>(nullptr), nullptr, saslMethod, "external");
  }

  const std::string address;
  int port;
  const std::string vhost;
  int maxChannels;
  int maxFrameSize;
  int heartbeat;
  std::chrono::seconds connectTimeout;
  std::chrono::seconds handshakeTimeout;
  amqp_table_t properties;
  amqp_sasl_method_enum saslMethod;

  amqp_connection_state_t connPtr;
  amqp_socket_t* socketPtr;
  struct timeval connectTv;
  struct timeval handshakeTv;

  const amqp_rpc_reply_t normalReply;
  MockAMQP amqp;
};

}} // namespace rmqcxx.unit_tests

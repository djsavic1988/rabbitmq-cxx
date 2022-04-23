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

#include "ConnectionTest.hpp"
#include <rmqcxx/Channel.hpp>

namespace rmqcxx { namespace unit_tests {

  struct ChannelTest : public ConnectionTest {

    ChannelTest() : ConnectionTest(), channelId(0), pConn(nullptr) {

    }

    Channel createSimpleChannel(amqp_channel_t chId = 9) {
      channelId = chId;
      EXPECT_CALL(amqp, maybe_release_buffers(connPtr))
        .Times(2);

      pConn = new Connection(createSimpleConnection());

      amqp_channel_open_ok_t openOk {};
      EXPECT_CALL(amqp, channel_open(connPtr, channelId))
        .WillOnce(::testing::Return(&openOk));
      EXPECT_CALL(amqp, get_rpc_reply(connPtr, "channel_open"))
        .WillOnce(::testing::Return(normalReply));

      Channel ch(*pConn, channelId);
      EXPECT_CALL(amqp, channel_close(connPtr, channelId, AMQP_REPLY_SUCCESS));
      EXPECT_CALL(amqp, get_rpc_reply(connPtr, "channel_close"))
        .WillOnce(::testing::Return(normalReply));
      return ch;
    }

    ~ChannelTest() override {
      delete pConn;
    }

    amqp_channel_t channelId;
    Connection* pConn;
  };
}} // namespace rmqcxx.unit_tests

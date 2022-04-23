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

#include <gtest/gtest.h>

#include <rmqcxx/Channel.hpp>

#include "ChannelTest.hpp"

namespace rmqcxx { namespace unit_tests {

using ::testing::_;
using ::testing::Pointee;
using ::testing::Return;

using std::move;
using std::string;

TEST_F(ChannelTest, Construction) {
  createSimpleChannel();
}

TEST_F(ChannelTest, FailureToOpenChannel) {
  amqp_channel_t channelId(9);
  auto conn = createSimpleConnection();
  amqp_channel_open_ok_t openOk {};

  EXPECT_CALL(amqp, channel_open(connPtr, channelId))
    .WillOnce(Return(&openOk));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "channel_open"))
    .WillOnce(Return(amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION }));

  EXPECT_CALL(amqp, error_string2(_))
    .WillOnce(Return("unit test library error"));

  EXPECT_THROW((Channel{conn, channelId}), rmqcxx::LibraryException);
}

TEST_F(ChannelTest, MoveConstructionTest) {
  auto ch = createSimpleChannel();
  auto ch2 = move(ch);
}

TEST_F(ChannelTest, Acknowledge) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(2);

  EXPECT_CALL(amqp, basic_ack(connPtr, channelId, 10UL, false))
    .WillOnce(Return(1));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_ack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.ack(10UL, false), 1);

  EXPECT_CALL(amqp, basic_ack(connPtr, channelId, 11UL, true))
    .WillOnce(Return(2));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_ack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.ack(11UL, true), 2);
}

TEST_F(ChannelTest, Nack) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(4);

  uint64_t tag(66);
  int rv = 1;
  EXPECT_CALL(amqp, basic_nack(connPtr, channelId, tag, false, false))
    .WillOnce(Return(rv));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_nack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.nack(tag, false, false), rv);

  ++tag;
  ++rv;
  EXPECT_CALL(amqp, basic_nack(connPtr, channelId, tag, false, true))
    .WillOnce(Return(rv));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_nack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.nack(tag, false, true), rv);

  ++tag;
  ++rv;
  EXPECT_CALL(amqp, basic_nack(connPtr, channelId, tag, true, false))
    .WillOnce(Return(rv));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_nack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.nack(tag, true, false), rv);

  ++tag;
  ++rv;
  EXPECT_CALL(amqp, basic_nack(connPtr, channelId, tag, true, true))
    .WillOnce(Return(rv));
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_nack"))
    .WillOnce(Return(normalReply));
  EXPECT_EQ(ch.nack(tag, true, true), rv);
}

TEST_F(ChannelTest, GenericRPC) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(2);

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc without args"))
    .WillOnce(Return(normalReply));
  bool called = false;
  ch.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel) {
    MockAMQP::instance()->lastRPCMethod = "rpc without args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    called = true;
  });
  EXPECT_TRUE(called);

  // rpc with args
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc with args"))
    .WillOnce(Return(normalReply));
  called = false;
  ch.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel, int arg) {
    MockAMQP::instance()->lastRPCMethod = "rpc with args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    EXPECT_EQ(arg, 99);
    called = true;
  }, 99);
  EXPECT_TRUE(called);
}

TEST_F(ChannelTest, QoS) {
  auto ch = createSimpleChannel();
  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(2);

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_qos"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  EXPECT_CALL(amqp, basic_qos(connPtr, channelId, 0, 10, 0));
  ch.qos(10);

  EXPECT_CALL(amqp, basic_qos(connPtr, channelId, 1, 11, 1));
  ch.qos(11, true, 1);
}

TEST_F(ChannelTest, Flow) {
  auto ch = createSimpleChannel();
  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(2);

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "channel_flow"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_channel_flow_ok_t chFlowResult{.active = true};

  EXPECT_CALL(amqp, channel_flow(connPtr, channelId, true))
    .WillOnce(Return(&chFlowResult));
  EXPECT_TRUE(ch.flow(true));

  EXPECT_CALL(amqp, channel_flow(connPtr, channelId, false))
    .WillOnce(Return(&chFlowResult));
  EXPECT_TRUE(ch.flow(false));
}

TEST_F(ChannelTest, Recover) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(2);

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_recover"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_basic_recover_ok_t result{0};

  EXPECT_CALL(amqp, basic_recover(connPtr, channelId, true))
    .WillOnce(Return(&result));
  ch.recover(true);

  EXPECT_CALL(amqp, basic_recover(connPtr, channelId, false))
    .WillOnce(Return(&result));
  ch.recover(false);
}

TEST_F(ChannelTest, Publish) {
  auto ch = createSimpleChannel();
  EXPECT_CALL(amqp, maybe_release_buffers_on_channel(connPtr, channelId))
    .Times(3);

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_publish"))
    .Times(3)
    .WillRepeatedly(Return(normalReply));

  amqp_basic_properties_t props{0};
  string exchange("exchange"), routingKey("routingKey"), body("body");
  EXPECT_CALL(amqp, basic_publish(connPtr, channelId, bytes(exchange), bytes(routingKey), 1, 1, Pointee(props), bytes(body)))
    .WillOnce(Return(AMQP_STATUS_OK));
  ch.publish(exchange, routingKey, true, true, body);

  exchange = "ex1";
  routingKey = "rk1";
  body = "body1";
  props = amqp_basic_properties_t{.priority = 10};
  EXPECT_CALL(amqp, basic_publish(connPtr, channelId, bytes(exchange), bytes(routingKey), 0, 0, Pointee(props), bytes(body)))
    .WillOnce(Return(AMQP_STATUS_OK));
  ch.publish(exchange, routingKey, false, false, body, props);

  exchange = "ex2";
  EXPECT_CALL(amqp, basic_publish(connPtr, channelId, bytes(exchange), bytes(routingKey), 0, 0, Pointee(props), bytes(body)))
    .WillOnce(Return(AMQP_STATUS_BAD_AMQP_DATA));
  EXPECT_THROW(ch.publish(exchange, routingKey, false, false, body, props), ChannelException);
}

}} // namespace rmqcxx.unit_tests

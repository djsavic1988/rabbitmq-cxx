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

#include "ChannelTest.hpp"
#include <rmqcxx/Queue.hpp>
#include <rmqcxx/TableEntry.hpp>

namespace rmqcxx { namespace unit_tests {

using ::testing::Return;

using std::move;
using std::string;

struct QueueTest : public ChannelTest {

};

TEST_F(QueueTest, Construction) {
  auto ch = createSimpleChannel();

  Queue q0(ch, "q0");
  EXPECT_EQ(q0.name(), "q0");

  Queue q1(move(q0));
  EXPECT_EQ(q1.name(), "q0");
}

TEST_F(QueueTest, GenericRPC) {
  auto ch = createSimpleChannel();

  Queue q(ch, "q0");

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc without args"))
    .WillOnce(Return(normalReply));
  bool called = false;
  q.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel, amqp_bytes_t queue) {
    MockAMQP::instance()->lastRPCMethod = "rpc without args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    EXPECT_EQ(string(static_cast<char*>(queue.bytes), queue.len), string("q0"));
    called = true;
  });
  EXPECT_TRUE(called);

  // rpc with args
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc with args"))
    .WillOnce(Return(normalReply));
  called = false;
  q.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel, amqp_bytes_t queue, int arg) {
    MockAMQP::instance()->lastRPCMethod = "rpc with args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    EXPECT_EQ(string(static_cast<char*>(queue.bytes), queue.len), string("q0"));
    EXPECT_EQ(arg, 99);
    called = true;
  }, 99);
  EXPECT_TRUE(called);
}

TEST_F(QueueTest, Declare) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "queue_declare"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  Queue q(ch, "q0");
  amqp_queue_declare_ok_t result{};
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())};

  EXPECT_CALL(amqp, queue_declare(connPtr, channelId, qname, 1, 1, 1, 1, amqp_table_t{0}))
    .WillOnce(Return(&result));
  EXPECT_EQ(q.declare(true, true, true, true), &result);

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, queue_declare(connPtr, channelId, qname, 0, 0, 0, 0, amqp_table_t{.num_entries = 1, .entries = &rawEntry }))
    .WillOnce(Return(&result));
  EXPECT_EQ(q.declare(false, false, false, false, TableEntry("entry", 1)), &result);
}

TEST_F(QueueTest, Bind) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "queue_bind"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  Queue q(ch, "q0");
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())},
    ex0 { .len = ::strlen("ex0"), .bytes = const_cast<char*>("ex0") },
    ex1 { .len = ::strlen("ex1"), .bytes = const_cast<char*>("ex1") },
    rkey0 { .len = ::strlen("rkey0"), .bytes = const_cast<char*>("rkey0") },
    rkey1 { .len = ::strlen("rkey1"), .bytes = const_cast<char*>("rkey1") };

  amqp_queue_bind_ok_t result{};
  EXPECT_CALL(amqp, queue_bind(connPtr, channelId, qname, ex0, rkey0, amqp_table_t{0}))
    .WillOnce(Return(&result));
  q.bind("ex0", "rkey0");

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, queue_bind(connPtr, channelId, qname, ex1, rkey1, amqp_table_t{ .num_entries = 1, .entries = &rawEntry}))
    .WillOnce(Return(&result));
  q.bind("ex1", "rkey1", TableEntry("entry", 1));
}

TEST_F(QueueTest, Unbind) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "queue_unbind"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  Queue q(ch, "q0");
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())},
    ex0 { .len = ::strlen("ex0"), .bytes = const_cast<char*>("ex0") },
    ex1 { .len = ::strlen("ex1"), .bytes = const_cast<char*>("ex1") },
    rkey0 { .len = ::strlen("rkey0"), .bytes = const_cast<char*>("rkey0") },
    rkey1 { .len = ::strlen("rkey1"), .bytes = const_cast<char*>("rkey1") };

  amqp_queue_unbind_ok_t result{};
  EXPECT_CALL(amqp, queue_unbind(connPtr, channelId, qname, ex0, rkey0, amqp_table_t{0}))
    .WillOnce(Return(&result));
  q.unbind("ex0", "rkey0");

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, queue_unbind(connPtr, channelId, qname, ex1, rkey1, amqp_table_t{ .num_entries = 1, .entries = &rawEntry}))
    .WillOnce(Return(&result));
  q.unbind("ex1", "rkey1", TableEntry("entry", 1));
}

TEST_F(QueueTest, Consume) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "basic_consume"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  Queue q(ch, "q0");
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())},
    ctag { .len = ::strlen("ctag"), .bytes = const_cast<char*>("ctag") };


  amqp_basic_consume_ok_t result0 { .consumer_tag = ctag },
    result1 { .consumer_tag = { .len = ::strlen("consumerTag"), .bytes = const_cast<char*>("consumerTag") }};
  EXPECT_CALL(amqp, basic_consume(connPtr, channelId, qname, ctag, 1, 1, 1, amqp_table_t{0}))
    .WillOnce(Return(&result0));
  EXPECT_EQ(q.consume("ctag", true, true, true), "ctag");

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, basic_consume(connPtr, channelId, qname, amqp_bytes_t{0}, 0, 0, 0, amqp_table_t{ .num_entries = 1, .entries = &rawEntry}))
    .WillOnce(Return(&result1));
  EXPECT_EQ(q.consume("", false, false, false, TableEntry("entry", 1)), "consumerTag");
}

TEST_F(QueueTest, Remove) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "queue_delete"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  Queue q(ch, "q0");
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())};

  amqp_queue_delete_ok_t result { .message_count = 55 };

  EXPECT_CALL(amqp, queue_delete(connPtr, channelId, qname, 1, 1))
    .WillOnce(Return(&result));
  EXPECT_EQ(q.remove(true, true), 55);
  EXPECT_CALL(amqp, queue_delete(connPtr, channelId, qname, 0, 0))
    .WillOnce(Return(&result));
  EXPECT_EQ(q.remove(false, false), 55);

}

TEST_F(QueueTest, Purge) {
  auto ch = createSimpleChannel();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "queue_purge"))
    .WillOnce(Return(normalReply));

  Queue q(ch, "q0");
  amqp_bytes_t qname {.len = q.name().size(), .bytes = const_cast<char*>(q.name().data())};

  amqp_queue_purge_ok_t result { .message_count = 82U};
  EXPECT_CALL(amqp, queue_purge(connPtr, channelId, qname))
    .WillOnce(Return(&result));
  EXPECT_EQ(q.purge(), 82U);
}

}} // namespace rmqcxx.unit_tests

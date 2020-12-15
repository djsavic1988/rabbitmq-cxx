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
#include <rmqcxx/Exchange.hpp>
#include <rmqcxx/TableEntry.hpp>

namespace rmqcxx { namespace unit_tests {

using ::testing::Return;

using std::string;

struct ExchangeTest : ChannelTest {

};

TEST_F(ExchangeTest, Construction) {
  auto ch = createSimpleChannel();
  Exchange ex(ch, "exchange");
  EXPECT_EQ(ex.name(), string("exchange"));

  Exchange ex2(std::move(ex));
  EXPECT_EQ(ex2.name(), string("exchange"));
}

TEST_F(ExchangeTest, GenericRPC) {
  auto ch = createSimpleChannel();

  Exchange ex(ch, "exchange1");

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc without args"))
    .WillOnce(Return(normalReply));
  bool called = false;
  ex.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel, amqp_bytes_t exchange) {
    MockAMQP::instance()->lastRPCMethod = "rpc without args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    EXPECT_EQ(string(static_cast<char*>(exchange.bytes), exchange.len), string("exchange1"));
    called = true;
  });
  EXPECT_TRUE(called);

  // rpc with args
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc with args"))
    .WillOnce(Return(normalReply));
  called = false;
  ex.rpc([&called, this] (::amqp_connection_state_t state, ::amqp_channel_t channel, amqp_bytes_t exchange, int arg) {
    MockAMQP::instance()->lastRPCMethod = "rpc with args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(channel, channelId);
    EXPECT_EQ(string(static_cast<char*>(exchange.bytes), exchange.len), string("exchange1"));
    EXPECT_EQ(arg, 99);
    called = true;
  }, 99);
  EXPECT_TRUE(called);
}

TEST_F(ExchangeTest, Declare) {
  auto ch = createSimpleChannel();

  Exchange ex(ch, "exchange1");

  amqp_exchange_declare_ok_t result{};
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "exchange_declare"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_bytes_t name = { .len = ex.name().size(), .bytes = const_cast<char*>( ex.name().data()) }, type = { .len = ::strlen("type"), .bytes = const_cast<char*>("type")};

  EXPECT_CALL(amqp, exchange_declare(connPtr, channelId, name, type, 0, 0, 0, 0, amqp_table_t{0}))
    .WillOnce(Return(&result));
  ex.declare("type", false, false, false);

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, exchange_declare(connPtr, channelId, name, type, 1, 1, 1, 0, amqp_table_t {.num_entries = 1, .entries = &rawEntry }))
    .WillOnce(Return(&result));
  ex.declare("type", true, true, true, TableEntry("entry", 1));
}

TEST_F(ExchangeTest, Bind) {
   auto ch = createSimpleChannel();

  Exchange ex(ch, "exchange1");

  amqp_exchange_bind_ok_t result{};
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "exchange_bind"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_bytes_t name = { .len = ex.name().size(), .bytes = const_cast<char*>( ex.name().data()) }, rkey = { .len = ::strlen("rkey"), .bytes = const_cast<char*>("rkey")}, otherName {.len = ::strlen("otherEx"), .bytes = const_cast<char*>("otherEx") };

  EXPECT_CALL(amqp, exchange_bind(connPtr, channelId, name, otherName, rkey, amqp_table_t{0}))
    .WillOnce(Return(&result));
  ex.bind("otherEx", "rkey");

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, exchange_bind(connPtr, channelId, name, otherName, rkey, amqp_table_t {.num_entries = 1, .entries = &rawEntry }))
    .WillOnce(Return(&result));
  ex.bind("otherEx", "rkey", TableEntry("entry", 1));
}

TEST_F(ExchangeTest, Unbind) {
  auto ch = createSimpleChannel();

  Exchange ex(ch, "exchange1");

  amqp_exchange_unbind_ok_t result{};
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "exchange_unbind"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_bytes_t name = { .len = ex.name().size(), .bytes = const_cast<char*>( ex.name().data()) }, rkey = { .len = ::strlen("rkey"), .bytes = const_cast<char*>("rkey")}, otherName {.len = ::strlen("otherEx"), .bytes = const_cast<char*>("otherEx") };

  EXPECT_CALL(amqp, exchange_unbind(connPtr, channelId, name, otherName, rkey, amqp_table_t{0}))
    .WillOnce(Return(&result));
  ex.unbind("otherEx", "rkey");

  amqp_table_entry_t rawEntry = {
    .key = amqp_bytes_t {.len = ::strlen("entry"), .bytes = const_cast<char*>("entry")},
    .value = {.kind = AMQP_FIELD_KIND_I32}
  };
  rawEntry.value.value.i32 = 1;
  EXPECT_CALL(amqp, exchange_unbind(connPtr, channelId, name, otherName, rkey, amqp_table_t {.num_entries = 1, .entries = &rawEntry }))
    .WillOnce(Return(&result));
  ex.unbind("otherEx", "rkey", TableEntry("entry", 1));
}

TEST_F(ExchangeTest, Remove) {
  auto ch = createSimpleChannel();
  Exchange ex(ch, "exchange1");
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "exchange_delete"))
    .Times(2)
    .WillRepeatedly(Return(normalReply));

  amqp_exchange_delete_ok_t result{};
  amqp_bytes_t name = { .len = ex.name().size(), .bytes = const_cast<char*>( ex.name().data()) };

  EXPECT_CALL(amqp, exchange_delete(connPtr, channelId, name, 1))
    .WillOnce(Return(&result));
  ex.remove(true);

  EXPECT_CALL(amqp, exchange_delete(connPtr, channelId, name, 0))
    .WillOnce(Return(&result));
  ex.remove(false);
}

}} // namespace rmqcxx.unit_tests

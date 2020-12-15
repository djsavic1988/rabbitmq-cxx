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

#include <rmqcxx/Message.hpp>

#include "MockAMQP.hpp"

namespace rmqcxx { namespace unit_tests {
  using ::testing::Test;

struct TestMessage : public AMQPStruct<::amqp_message_t> {
  using AMQPStruct<::amqp_message_t>::AMQPStruct;
  inline bool moved() const noexcept { return moved_; }
};

struct MessageTest : public Test {
  bool moved(const Message& e) const noexcept {
    return reinterpret_cast<const TestMessage&>(e).moved();
  }
  MockAMQP amqp;
};

TEST_F(MessageTest, DefaultConstructionTest) {
  Message x;
  EXPECT_EQ(moved(x), false);
  EXPECT_CALL(amqp, destroy_message(static_cast<::amqp_message_t*>(x)));
}

TEST_F(MessageTest, MoveConstructionTest) {
  Message x;
  x->body.bytes = const_cast<char*>("10");
  Message y(std::move(x));
  EXPECT_EQ(moved(x), true);
  EXPECT_EQ(moved(y), false);
  EXPECT_EQ(y->body.bytes, "10");
  EXPECT_CALL(amqp, destroy_message(static_cast<::amqp_message_t*>(y)));
}

TEST_F(MessageTest, MoveAssignmentTest) {
  Message x;
  x->body.bytes = const_cast<char*>("10");
  Message y;
  y = (std::move(x));
  EXPECT_EQ(moved(x), true);
  EXPECT_EQ(moved(y), false);
  EXPECT_EQ(y->body.bytes, "10");
  EXPECT_CALL(amqp, destroy_message(static_cast<::amqp_message_t*>(y)));
}

TEST_F(MessageTest, CastTest) {
  Message x;
  x->body.bytes = const_cast<char*>("10");
  const auto& v = static_cast<const ::amqp_message_t&>(x).body.bytes;
  EXPECT_EQ(v, "10");
  static_cast<::amqp_message_t&>(x).body.bytes = const_cast<char*>("14");
  EXPECT_EQ(v, "14");
  EXPECT_CALL(amqp, destroy_message(static_cast<::amqp_message_t*>(x)));
}

TEST_F(MessageTest, CompatibilityTest) {
  Message x;
  auto& base = static_cast<AMQPStruct<::amqp_message_t>& >(x);
  base->body.bytes = const_cast<char*>("12");
  EXPECT_EQ(x->body.bytes, "12");
  EXPECT_CALL(amqp, destroy_message(static_cast<::amqp_message_t*>(x)));
}

}} // namespace rmqcxx.unit_tests

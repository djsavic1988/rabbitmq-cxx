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

#include "MockAMQP.hpp"
#include <rmqcxx/ReturnedMessage.hpp>


namespace rmqcxx { namespace unit_tests {

using ::testing::_;
using ::testing::Test;

using std::move;
using std::string;

struct ReturnedMessageTest : public Test {
  MockAMQP amqp;
};

TEST_F(ReturnedMessageTest, Construction) {

  EXPECT_CALL(amqp, destroy_message(_));
  ReturnedMessage msg(Message(), amqp_basic_return_t{});

  ReturnedMessage other(move(msg));

  ReturnedMessage third(Message(), amqp_basic_return_t{});
  third = move(other);
}

TEST_F(ReturnedMessageTest, Accessors) {
  EXPECT_CALL(amqp, destroy_message(_));
  ReturnedMessage msg(Message(amqp_message_t {.body = amqp_bytes_t {.len = 4, .bytes = const_cast<char*>("body")}}), amqp_basic_return_t{.reply_code = 75});
  EXPECT_EQ(string(static_cast<char*>(msg.message()->body.bytes), msg.message()->body.len), "body");
  EXPECT_EQ(msg.method().reply_code, 75);
}

}} // namespace rmqcxx.unit_tests

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

#include <rmqcxx/Envelope.hpp>

#include "MockAMQP.hpp"

namespace rmqcxx { namespace unit_tests {

using ::testing::Test;

struct TestEnvelope : public AMQPStruct<::amqp_envelope_t> {
  using AMQPStruct<::amqp_envelope_t>::AMQPStruct;
  inline bool moved() const noexcept { return moved_; }
};

struct EnvelopeTest : public Test {
  bool moved(const Envelope& e) const noexcept {
    return reinterpret_cast<const TestEnvelope&>(e).moved();
  }
  MockAMQP amqp;
};

TEST_F(EnvelopeTest, DefaultConstructionTest) {
  Envelope x;
  EXPECT_EQ(moved(x), false);
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(x)));
}

TEST_F(EnvelopeTest, MoveConstructionTest) {
  Envelope x;
  x->channel = 10;
  Envelope y(std::move(x));
  EXPECT_EQ(moved(x), true);
  EXPECT_EQ(moved(y), false);
  EXPECT_EQ(y->channel, 10);
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(y)));
}

TEST_F(EnvelopeTest, MoveAssignmentTest) {
  Envelope x;
  x->channel = 10;
  Envelope y;
  y = (std::move(x));
  EXPECT_EQ(moved(x), true);
  EXPECT_EQ(moved(y), false);
  EXPECT_EQ(y->channel, 10);
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(y)));
}

TEST_F(EnvelopeTest, CastTest) {
  Envelope x;
  x->channel = 10;
  const auto& v = static_cast<const ::amqp_envelope_t&>(x).channel;
  EXPECT_EQ(v, 10);
  static_cast<::amqp_envelope_t&>(x).channel = 14;
  EXPECT_EQ(v, 14);
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(x)));
}

TEST_F(EnvelopeTest, CompatibilityTest) {
  Envelope x;
  auto& base = static_cast<AMQPStruct<::amqp_envelope_t>& >(x);
  base->channel = 12;
  EXPECT_EQ(x->channel, 12);
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(x)));
}

TEST_F(EnvelopeTest, BodyGetterTest) {
  Envelope x;
  x->message.body.bytes = const_cast<char*>("123");
  x->message.body.len = 3;
  EXPECT_EQ(x.body(), "123");
  EXPECT_CALL(amqp, destroy_envelope(static_cast<::amqp_envelope_t*>(x)));
}

}} // namespace rmqcxx.unit_tests

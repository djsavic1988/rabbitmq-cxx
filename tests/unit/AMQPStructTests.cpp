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

#include <utility>

#include <gtest/gtest.h>

#include <rmqcxx/AMQPStruct.hpp>


namespace rmqcxx { namespace unit_tests {

using std::pair;

using ::testing::Test;

struct AMQPStructTest : public Test {

};

struct TestAMQPStruct : public AMQPStruct<pair<int, int> > {
  using AMQPStruct<pair<int, int> >::AMQPStruct;
  inline bool moved() noexcept { return moved_; }
};

TEST_F(AMQPStructTest, DefaultConstructionTest) {
  TestAMQPStruct x;
  EXPECT_EQ(x.moved(), false);
}

TEST_F(AMQPStructTest, MoveConstructionTest) {
  TestAMQPStruct x;
  x->first = 10;
  x->second = 11;
  TestAMQPStruct y(std::move(x));
  EXPECT_EQ(x.moved(), true);
  EXPECT_EQ(y.moved(), false);
  EXPECT_EQ(y->first, 10);
  EXPECT_EQ(y->second, 11);
}

TEST_F(AMQPStructTest, MoveAssignmentTest) {
  TestAMQPStruct x;
  x->first = 10;
  x->second = 11;
  TestAMQPStruct y;
  y = std::move(x);
  EXPECT_EQ(x.moved(), true);
  EXPECT_EQ(y.moved(), false);
  EXPECT_EQ(y->first, 10);
  EXPECT_EQ(y->second, 11);
}

TEST_F(AMQPStructTest, CastTest) {
  TestAMQPStruct x;
  x->first = 10;
  x->second = 11;
  const auto& v = static_cast<const pair<int, int>&>(x).first;
  EXPECT_EQ(v, 10);
  static_cast<pair<int, int>&>(x).first = 14;
  EXPECT_EQ(v, 14);
}

}} // namespace rmqcxx.unit_tests

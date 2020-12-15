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

#include <amqp.h>

#include "AMQPStruct.hpp"

namespace rmqcxx {

/**
 * Wrapper around ::amqp_message_t
  */
class Message final : public AMQPStruct<::amqp_message_t> {
public:
  /**
   * Constructor
   */
  explicit Message(::amqp_message_t msg = ::amqp_message_t{0}) noexcept : AMQPStruct(std::move(msg)) {}

  /**
   * Destructor
   */
  ~Message() noexcept {
    if (!moved_)
      ::amqp_destroy_message(&memory_);
  }

  /**
   * Not copy constructable
   */
  Message(const Message&) = delete;

  /**
   * Move constructable
   */
  Message(Message&&) noexcept = default;

  /**
   * Can't be copy assigned
   */
  Message& operator=(const Message&) = delete;

  /**
   * Move assignable
   */
  Message& operator=(Message&&) noexcept = default;

};

} // namespace rmqcxx

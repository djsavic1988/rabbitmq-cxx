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

#include "Message.hpp"

namespace rmqcxx {

/**
 * Class that handles messages NACKed (returned) by the broker
  */
class ReturnedMessage final {
public:

  /**
   * Constructor
   *
   * @param[in] message AMQP message
   * @param[in] method AMQP basic return method structure
   */
  ReturnedMessage(Message message, ::amqp_basic_return_t method) noexcept :
    message_(std::move(message)),
    method_(std::move(method)) {}

  /**
   * Destructor
   */
  ~ReturnedMessage() noexcept = default;

  /**
   * Can't be copy constructed
   */
  ReturnedMessage(const ReturnedMessage&) = delete;

  /**
   * Move constructable
   */
  ReturnedMessage(ReturnedMessage&&) noexcept = default;

  /**
   * Can't be copy assigned
   */
  ReturnedMessage& operator=(const ReturnedMessage&) = delete;

  /**
   * Move assignable
   */
  ReturnedMessage& operator=(ReturnedMessage&&) noexcept = default;

  /**
   * Reference to the message
   * @return Reference to the message
   */
  const Message& message() const noexcept {
    return message_;
  }

  /**
   * Refererence to the return method information
   * @return Refererence to the return method information
   */
  const ::amqp_basic_return_t& method() const noexcept {
    return method_;
  }

private:

  /**
   * Storage for the message
   */
  Message message_;

  /**
   * Storage for the method information
   */
  ::amqp_basic_return_t method_;
};

} // namespace rmqcxx

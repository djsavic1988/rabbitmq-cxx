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
 * Wraps ::amqp_envelope_t
*/
class Envelope final : public AMQPStruct<::amqp_envelope_t> {
public:
  /**
   * Default constructor
   *
   * @param v AMQP envelope
   */
  explicit Envelope(::amqp_envelope_t v = ::amqp_envelope_t()) noexcept : AMQPStruct<::amqp_envelope_t>(std::move(v)) {};

  /**
   * Destructor
   */
  ~Envelope() noexcept {
    if (!moved_)
      ::amqp_destroy_envelope(&memory_);
  }

  /**
   * Can't be copy constructed
   */
  Envelope(const Envelope&) = delete;

  /**
   * Move constructable
   */
  Envelope(Envelope&&) noexcept = default;

  /**
   * Can't be copy assigned
   */
  Envelope& operator=(const Envelope&) = delete;

  /**
   * Move assignable
   */
  Envelope& operator=(Envelope&&) noexcept = default;

  /**
   * Envelope content provider
   * @return Envelope content
   */
  std::string body() const noexcept {
    const auto& b = static_cast<::amqp_envelope_t>(memory_).message.body;
    return std::string(static_cast<const char*>(b.bytes), b.len);
  }
};

} // namespace rmqcxx

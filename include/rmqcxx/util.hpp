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

#include <cstring>
#include <chrono>
#include <functional>

#include <amqp.h>

namespace rmqcxx {

/**
 * Converts STL like sequential containers to AMQP bytes
 * @tparam Container STL like container with sequential memory layout, has to have size and data
 * @return An instance of amqp_bytes_t
 */
template <typename Container>
static ::amqp_bytes_t bytes(const Container& container) noexcept {
  return ::amqp_bytes_t { container.size(), const_cast<typename Container::value_type*>(container.data()) };
}

/**
 * Converts C string to AMQP bytes
 * @return An instance of amqp_bytes_t
 */
static ::amqp_bytes_t bytes(const char* v) noexcept {
  return ::amqp_bytes_t {
    .len = ::strlen(v),
    .bytes = const_cast<char*>(v)
  };
}

/**
 * Constructs an STL like container from AMQP bytes
 * @tparam Container STL like container that has value_type and ctor(const value_type*, size_t len)
 * @return Instance of the desired container
 */
template <typename Container>
static Container container(const ::amqp_bytes_t& bytes) {
  return Container(static_cast<const typename Container::value_type*>(bytes.bytes), bytes.len);
}

/**
 * Converts duration to timeval
 * @tparam Duration Anything std::chrono::duration compatible
 * @return An instance of ::timeval
 */
template <typename Duration>
static ::timeval timeValue(const Duration& duration) noexcept
{
  const auto& seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  return ::timeval {
    .tv_sec = seconds.count(),
    .tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(duration - seconds).count()
  };
}

namespace impl {

  /**
   * Close method decoder
   * @tparam T amqp_channel_close_t compatible method
   * @return Decoded method information as a string
   */
  template <typename T>
  std::string decodeCloseMethod(const T* decoded) {
    return std::string("Code: ") + std::to_string(decoded->reply_code) + " Message: " + container<std::string>(decoded->reply_text);
  }
} // namespace impl

/**
 * Decodes amqp_channel_close_t to a string
 */
inline std::string decodeAmqpMethod(const ::amqp_channel_close_t* decoded) {
  return impl::decodeCloseMethod(decoded);
}

/**
 * Decodes amqp_connection_close_t to a string
 */
inline std::string decodeAmqpMethod(const ::amqp_connection_close_t* decoded) {
  return impl::decodeCloseMethod(decoded);
}

/**
 * Helper class to execute a std::function compatible object after the defer object goes out of scope
 */
class defer final {
public:
  /**
   * Constructor
   *
   * @tparam Function Anything that can be bound and that doesn't return anythin
   * @tparam Args Arguments for the deferred function
   *
   * @param[in] f Function to call upon destruction
   * @param[in] args Arguments for the function
   */
  template <typename Function, typename... Args>
  explicit defer(Function f, Args&&... args) noexcept :
    function(std::bind(f, std::forward<Args>(args)...)) {}

  /**
   * Calls the functon that was passed in the constructor
    */
  ~defer() noexcept(false) {  // without imlicitly setting noexcept(false) it happens in tests that the exception is not caught
    if (function)
      function();
  }

  /**
   * Can't be copy constructed
   */
  defer(const defer&) = delete;

  /**
   * Can't be move constructed
   */
  defer(defer&&) noexcept = delete;

  /**
   * Can't be copy assigned
   */
  defer& operator=(const defer&) = delete;

  /**
   * Can't be move assigned
   */
  defer& operator=(defer&&) noexcept = delete;

private:
  /**
   * Function storage
   */
  std::function<void()> function;
};

} // namespace rmqcxx

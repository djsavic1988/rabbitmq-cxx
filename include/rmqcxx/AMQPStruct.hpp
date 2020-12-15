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

namespace rmqcxx {

/**
 * Helper structure to manage libamqp-c structures
 * @tparam T amqp_* type of struct
 */
template <typename T>
class AMQPStruct {
public:

  /**
   * Constructor
   *
   * @param[in] memory Memory to store
   */
  explicit AMQPStruct(T memory = T()) noexcept : memory_(std::move(memory)), moved_(false) {}

  /**
   * Not copy constructable
   */
  AMQPStruct(const AMQPStruct<T>&) = delete;

  /**
   * Move constructor
   */
  AMQPStruct(AMQPStruct&& other) noexcept : memory_(std::move(other.memory_)), moved_(false) { other.moved_ = true; }

  /**
   * Can't be copy assigned
   */
  AMQPStruct& operator=(const AMQPStruct<T>&) = delete;

  /**
   * Move assignable
   */
  AMQPStruct& operator=(AMQPStruct&& other) noexcept {
    this->memory_ = std::move(other.memory_);
    this->moved_ = false;
    other.moved_ = true;
    return *this;
  }

  /**
   * Direct access to memory via a cast
   */
  inline operator T&() noexcept {
    return memory_;
  }

  /**
   * Direct access to memory via a cast
   */
  inline operator const T&() const noexcept {
    return memory_;
  }

  /**
   * Direct access to memory via a cast
   */
  inline operator T*() noexcept {
    return &memory_;
  }

  /**
   * Pointer access to memory via a cast
   */
  inline operator const T*() const noexcept {
    return &memory_;
  }

  /**
   * Overloaded -> operator to access the underlying structure
   *
   */
  inline T* operator ->() noexcept {
    return &memory_;
  }

  /**
   * Overloaded -> operator to access the underlying structure
   */
  inline const T* operator->() const noexcept {
    return &memory_;
  }

protected:
  /**
   * Memory storage
   */
  T memory_;

  /**
   * Flag that tells us if the underlying object has been moved
   */
  bool moved_;
};

} // namespace rmqcxx

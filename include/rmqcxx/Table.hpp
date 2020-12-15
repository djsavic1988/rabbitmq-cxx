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

#include <vector>

#include <amqp.h>

namespace rmqcxx {

/**
 * Wrapper around amqp_table_t
 */
class Table final {
public:

  /**
   * Constructs a table from table entries
   *
   * @tparam Args TableEntry convertible types
   * @param[in] args Table entries
   */
  template <typename... Args>
  Table(Args&&... args) noexcept :
    cache_{ static_cast<::amqp_table_entry_t>(args)... },
    value_{ .num_entries = sizeof...(args), .entries = const_cast<::amqp_table_entry_t*>(cache_.data()) } {
  }

  /**
   * Destructor
   */
  ~Table() noexcept = default;

  /**
   * Copy constructor
   */
  Table(const Table& other) :
    cache_(other.cache_),
    value_ {.num_entries = static_cast<int>(cache_.size()), .entries = cache_.data()} {}

  /**
   * Move constructor
   */
  Table(Table&& other) noexcept :
    cache_(std::move(other.cache_)),
    value_ {.num_entries = static_cast<int>(cache_.size()), .entries = cache_.data()} {}

  /**
   * Copy assignment operator
   */
  Table& operator=(const Table& other) {
    cache_ = other.cache_;
    value_.num_entries = cache_.size();
    value_.entries = cache_.data();
    return *this;
  }

  /**
   * Move assignment operator
   */
  Table& operator=(Table&& other) noexcept {
    cache_ = std::move(other.cache_);
    value_.num_entries = cache_.size();
    value_.entries = cache_.data();
    return *this;
  }

  /**
   * Conversion to amqp_table_t const reference
   */
  operator const amqp_table_t() const noexcept {
    return value_;
  }

private:

  /**
   * Storage for entries
   */
  std::vector<::amqp_table_entry_t> cache_;

  /**
   * Table cache
   */
  ::amqp_table_t value_;
};

} // namespace rmqxx

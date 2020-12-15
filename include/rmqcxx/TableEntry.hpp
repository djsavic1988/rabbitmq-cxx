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

#include <string>

#include <amqp.h>

#include "FieldValue.hpp"
#include "Table.hpp"
#include "util.hpp"

namespace rmqcxx {

/**
 * Represents a single table entry
 * @tparam T Any ::amqp_field_value_t convertible type
 */
class TableEntry final {
public:
  /**
   * Constructor
   *
   * @param[in] key Name of the table entry
   * @param[in] value Value of the table entry
   */
  TableEntry(std::string key, FieldValue value) noexcept : key_(std::move(key)), value_(std::move(value)) {}

  /**
   * Constructor
   * @param[in] entry AMQP Table entry
   */
  TableEntry(::amqp_table_entry_t entry) noexcept : key_(container<std::string>(entry.key)), value_(FieldValue(entry.value)) {}

  /**
   * Destructor
   */
  ~TableEntry() noexcept = default;

  /**
   * Can't be copy constructed
   */
  TableEntry(const TableEntry&) = delete;

  /**
   * Move constructable
   */
  TableEntry(TableEntry&&) noexcept = default;

  /**
   * Can't be copy assigned
   */
  TableEntry& operator=(const TableEntry&) = delete;

  /**
   * Can't be move assigned
   */
  TableEntry& operator=(TableEntry&&) noexcept = delete;

  /**
   * Conversion to amqp_table_entry_t
   */
  operator ::amqp_table_entry_t() noexcept {
    ::amqp_table_entry_t r;
    r.key = bytes(key_);
    r.value = static_cast<::amqp_field_value_t>(value_);
    return r;
  }
private:

  /**
   * Reference to the table entry name
   */
  std::string key_;

  /**
   * Reference to the table entry value
   */
  FieldValue value_;
};
} // namespace rmqcxx

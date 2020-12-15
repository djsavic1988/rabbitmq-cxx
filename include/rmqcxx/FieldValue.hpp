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

#include "Table.hpp"
#include "util.hpp"

namespace rmqcxx {

/**
 * Wraps ::amqp_field_value_t
 */
class FieldValue final {
public:
  /**
   * Constructs field value from bool
   * @param[in] v Value
   */
  FieldValue(bool v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_BOOLEAN, .value  = { .boolean = v }} {}

  /**
   * Constructs field value from 8 bit signed integer
   * @param[in] v Value
   */
  FieldValue(int8_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_I8, .value  = { .i8 = v }} {}

  /**
   * Constructs field value from 8 bit unsigned integer
   * @param[in] v Value
   */
  FieldValue(uint8_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_U8, .value  = { .u8 = v }} {}

  /**
   * Constructs field value from 16 bit signed integer
   * @param[in] v Value
   */
  FieldValue(int16_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_I16, .value  = { .i16 = v }} {}

  /**
   * Constructs field value from 16 bit unsigned integer
   * @param[in] v Value
   */
  FieldValue(uint16_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_U16, .value  = { .u16 = v }} {}

  /**
   * Constructs field value from 32 bit signed integer
   * @param[in] v Value
   */
  FieldValue(int32_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_I32, .value  = { .i32 = v }} {}

  /**
   * Constructs field value from 32 bit unsigned integer
   * @param[in] v Value
   */
  FieldValue(uint32_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_U32, .value  = { .u32 = v }} {}

  /**
   * Constructs field value from 64 bit signed integer
   * @param[in] v Value
   */
  FieldValue(int64_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_I64, .value  = { .i64 = v }} {}

  /**
   * Constructs field value from 64 bit unsigned integer
   * @param[in] v Value
   */
  FieldValue(uint64_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_U64, .value  = { .u64 = v }} {}

  /**
   * Constructs field value from float
   * @param[in] v Value
   */
  FieldValue(float v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_F32, .value  = { .f32 = v }} {}

  /**
   * Constructs field value from double
   * @param[in] v Value
   */
  FieldValue(double v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_F64, .value  = { .f64 = v }} {}

  /**
   * Constructs field value from ::amqp_decimal_t
   * @param[in] v Value
   */
  FieldValue(::amqp_decimal_t v) noexcept : cache_{ .kind = AMQP_FIELD_KIND_DECIMAL, .value  = { .decimal = v }} {}

  /**
   * Constructs field value from a string
   * @param[in] v Value
   * @param[in] kind Defaults to UTF-8, can be overriden if needed to AMQP_FIELD_KIND_BYTES
   */
  FieldValue(const std::string& v, uint8_t kind = AMQP_FIELD_KIND_UTF8) noexcept : cache_{ .kind = kind, .value  = { .bytes = bytes(v) }} {}

  /**
   * Constructs field value from a const char*
   * @param[in] v Value
   * @param[in] kind Defaults to UTF-8, can be overriden if needed to AMQP_FIELD_KIND_BYTES
   */
  FieldValue(const char* v, uint8_t kind = AMQP_FIELD_KIND_UTF8) : FieldValue(std::string(v), kind) {}

  /**
   * Constructs field value from 8 bit signed STL container with sequential memory
   * @tparam Container type of STL like container with sequential memory
   * @param[in] v Value
   */
  template <typename Container>
  FieldValue(const Container& v, typename std::enable_if<(sizeof(typename Container::value_type) == 1), void>::type* = nullptr) :
    cache_ { .kind = AMQP_FIELD_KIND_BYTES, .value = { .bytes = bytes(v) }} {}

    /**
   * Constructs field value from a table
   * @param[in] v Value
   */
  FieldValue(::amqp_table_t v) : cache_{ .kind = AMQP_FIELD_KIND_TABLE, .value = { .table = v }} {}

  /**
   * Constructs field value from a table
   * @param[in] v Value
   */
  FieldValue(const Table& v) : FieldValue(static_cast<::amqp_table_t>(v)) {}

  /**
   * Constructs field value from 8 bit signed integer
   * @tparam Container STL type of container with sequential memory
   * @param[in] v Value
   */
  template <typename Container>
  FieldValue(const Container& v, typename std::enable_if<std::is_same<typename Container::value_type, ::amqp_field_value_t>::value>::type* = nullptr) : cache_ {.kind = AMQP_FIELD_KIND_ARRAY, .value = {
    .array = amqp_array_t { .num_entries = static_cast<int>(v.size()), .entries = const_cast<amqp_field_value_t*>(v.data())}
  }} {}

  /**
   * Constructs from ::amqp_field_value_t
   */
  FieldValue(::amqp_field_value_t value) : cache_(value) {}

  /**
   * Destructor
   */
  ~FieldValue() noexcept = default;

  /**
   * Copy constructable
   */
  FieldValue(const FieldValue&) = default;

  /**
   * Move constructable
   */
  FieldValue(FieldValue&&) = default;

  /**
   * Copy assignable
   */
  FieldValue& operator=(const FieldValue&) = default;

  /**
   * Move assignable
   */
  FieldValue& operator=(FieldValue&&) = default;
  /**
   * Conversion to field value reference
   * @param v Value
   */
  inline operator const ::amqp_field_value_t&() const noexcept {
    return cache_;
  }

  /**
   * Conversion to field value reference
   */
  inline operator ::amqp_field_value_t&() noexcept {
    return cache_;
  }

private:

  /**
   * Cache for the conversion
   */
  ::amqp_field_value_t cache_;
};
} // namespace rmqcxx

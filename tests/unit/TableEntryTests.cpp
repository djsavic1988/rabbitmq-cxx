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

#include "comparison.hpp"

#include <rmqcxx/TableEntry.hpp>


namespace rmqcxx { namespace unit_tests {

using ::testing::Test;

using std::move;
using std::string;
using std::vector;

struct TableEntryTest : public Test {
};

static ::amqp_bytes_t bytes(const char* v) noexcept {
  return ::amqp_bytes_t {
    .len = ::strlen(v),
    .bytes = const_cast<char*>(v)
  };
}

TEST_F(TableEntryTest, Construction) {
  TableEntry t0("key", 1);
  TableEntry t1(move(t0));
}

TEST_F(TableEntryTest, Conversions) {
  amqp_table_entry_t entry = {
    .key = bytes("key0"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_BOOLEAN, .value = { .boolean = 1 } }
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key0", true)), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key1"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_I8, .value = { .i8 = 14 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key1", int8_t(14))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key2"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_U8, .value = { .u8 = 15 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key2", uint8_t(15))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key3"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_I16, .value = { .i16 = 16 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key3", int16_t(16))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key4"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_U16, .value = { .u16 = 17 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key4", uint16_t(17))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key5"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_I32, .value = { .i32 = 18 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key5", int32_t(18))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key6"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_U32, .value = { .u32 = 19 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key6", uint32_t(19))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key7"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_I64, .value = { .i64 = 20 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key7", int64_t(20))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key8"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_U64, .value = { .u64 = 21 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key8", uint64_t(21))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key9"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_F32, .value = { .f32 = 22.f }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key9", 22.f)), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key10"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_F64, .value = { .f64 = 23.0 }}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key10", 23.0)), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key11"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_DECIMAL, .value = { .decimal = amqp_decimal_t {24, 25}}}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key11", amqp_decimal_t{24, 25})), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key12"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_UTF8, .value = { .bytes = bytes("value") } }
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key12", string("value"))), entry);

  entry = amqp_table_entry_t {
    .key = bytes("key13"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_BYTES, .value = { .bytes = amqp_bytes_t { .len = 2, .bytes = const_cast<char*>("\x1a\x1b")}}}
  };
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key13", vector<uint8_t> {26, 27})), entry);

  amqp_table_entry_t tableEntries = { .key = bytes("key15"), .value = {
    amqp_field_value_t { .kind = AMQP_FIELD_KIND_I32, .value = { .i32 = 28 }}}};

  entry = amqp_table_entry_t {
    .key = bytes("key14"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_TABLE, .value = { .table =  amqp_table_t { .num_entries = 1, .entries = &tableEntries }}}
  };

  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry(string("key14"), Table(TableEntry(string("key15"), int32_t(28))))), entry);

  amqp_field_value_t values = {
    .kind = AMQP_FIELD_KIND_I32, .value = { .i32 = 29 }
  };
  entry = amqp_table_entry_t {
    .key = bytes("key16"),
    .value = amqp_field_value_t { .kind = AMQP_FIELD_KIND_ARRAY, .value = { .array = amqp_array_t { .num_entries = 1, .entries = &values}}}
  };
  std::vector<amqp_field_value_t> entries{{
    FieldValue(29)
  }};
  EXPECT_EQ(static_cast<amqp_table_entry_t>(TableEntry("key16", entries)), entry);
}


}} // namespace rmqcxx.unit_tests

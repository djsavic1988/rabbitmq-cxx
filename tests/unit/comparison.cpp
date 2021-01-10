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

#include "comparison.hpp"

#include <chrono>
#include <cstring>

using std::chrono::microseconds;

bool operator==(const struct timeval& l, const struct timeval& r) {
  // in order to support close matches because of internal casts the resolution in 100us

  microseconds lv(l.tv_usec + l.tv_sec * 1000000UL);
  microseconds rv(r.tv_usec + r.tv_sec * 1000000UL);

  return microseconds(100) > (lv > rv ? lv - rv : rv - lv);
}

bool operator==(const amqp_bytes_t& l, const amqp_bytes_t& r) {
  return l.len == r.len && 0 == ::memcmp(l.bytes, r.bytes, l.len);
}

bool operator==(const amqp_method_t& l, const amqp_method_t& r) {
  return l.id == r.id && l.decoded == r.decoded;
}

bool operator==(const amqp_rpc_reply_t& l, const amqp_rpc_reply_t& r) {
  return l.reply_type == r.reply_type && l.library_error == r.library_error && l.reply == r.reply;
}

bool operator==(const amqp_basic_return_t& l, const amqp_basic_return_t& r) {
  return l.reply_code == r.reply_code && l.reply_text == r.reply_text && l.exchange == r.exchange && l.routing_key == r.routing_key;
}

bool operator==(const amqp_frame_t& l, const amqp_frame_t& r) {
  if (l.channel != r.channel || l.frame_type != r.frame_type)
    return false;
  switch (l.frame_type) {
    case AMQP_FRAME_METHOD:
      return l.payload.method == r.payload.method;
    case AMQP_FRAME_BODY:
      return l.payload.body_fragment == r.payload.body_fragment;
    case AMQP_FRAME_HEADER:
      return l.payload.properties.class_id == r.payload.properties.class_id
        && l.payload.properties.body_size == r.payload.properties.body_size
        && l.payload.properties.decoded == r.payload.properties.decoded
        && l.payload.properties.raw == r.payload.properties.raw;
    default:
      return 0 == memcmp(&l.payload, &r.payload, sizeof(l.payload));
  }
}

bool operator==(const amqp_field_value_t& l, const amqp_field_value_t& r) {

  if (l.kind != r.kind) {
    return false;
  }

  switch(l.kind) {
    case AMQP_FIELD_KIND_DECIMAL:
      return 0 == memcmp(&l.value.decimal, &r.value.decimal, sizeof(l.value.decimal));
    case AMQP_FIELD_KIND_BYTES:
    case AMQP_FIELD_KIND_UTF8:
      return l.value.bytes == r.value.bytes;
    case AMQP_FIELD_KIND_TABLE:
      return l.value.table == r.value.table;
    case AMQP_FIELD_KIND_ARRAY:
      if (l.value.array.num_entries != r.value.array.num_entries)
        return false;
      for (decltype(l.value.array.num_entries) i = 0; i < l.value.array.num_entries; ++i) {
        if (!(l.value.array.entries[i] == r.value.array.entries[i]))
          return false;
      }
      return true;
    case AMQP_FIELD_KIND_BOOLEAN:
      return l.value.boolean == r.value.boolean;
    case AMQP_FIELD_KIND_I8:
      return l.value.i8 == r.value.i8;
    case AMQP_FIELD_KIND_U8:
      return l.value.u8 == r.value.u8;
    case AMQP_FIELD_KIND_I16:
      return l.value.i16 == r.value.i16;
    case AMQP_FIELD_KIND_U16:
      return l.value.u16 == r.value.u16;
    case AMQP_FIELD_KIND_I32:
      return l.value.i32 == r.value.i32;
    case AMQP_FIELD_KIND_U32:
      return l.value.u32 == r.value.u32;
    case AMQP_FIELD_KIND_I64:
      return l.value.i64 == r.value.i64;
    case AMQP_FIELD_KIND_U64:
      return l.value.u64 == r.value.u64;
    case AMQP_FIELD_KIND_F32:
      return l.value.f32 == r.value.f32;
    case AMQP_FIELD_KIND_F64:
      return l.value.f64 == r.value.f64;
    default:
      return 0 == memcmp(&l.value, &r.value, sizeof(l.value));
  }
}

bool operator==(const amqp_table_entry_t& l, const amqp_table_entry_t& r) {
  return l.key == r.key && l.value == r.value;
}

bool operator==(const amqp_table_t& l, const amqp_table_t& r) {
  if (l.num_entries != r.num_entries)
    return false;
  for (decltype(l.num_entries) i = 0; i < l.num_entries; ++i) {
    if (!(l.entries[i] == r.entries[i]))
      return false;
  }
  return true;
}

bool operator==(const amqp_basic_properties_t& l, const amqp_basic_properties_t& r) {
  return l._flags == r._flags &&  l.content_type == r.content_type && l.content_encoding == r.content_encoding && l.headers == r.headers && l.delivery_mode == r.delivery_mode && l.priority == r.priority && l.correlation_id == r.correlation_id && l.reply_to == r.reply_to && l.expiration == r.expiration && l.message_id == r.message_id && l.timestamp == r.timestamp && l.type == r.type && l.user_id == r.user_id && l.app_id == r.app_id && l.cluster_id == r.cluster_id;
}

bool operator==(const amqp_message_t& l, const amqp_message_t& r) {
  return l.properties == r.properties && l.body == r.body;
}

#include <ostream>

using std::ostream;
using std::string;

ostream& operator<<(ostream& os, const struct timeval& v) {
  return os << "tv_sec: " << v.tv_sec << " tv_usec: " << v.tv_usec;
}

ostream& operator<<(ostream& os, const amqp_bytes_t& v) {
  return os << '(' << v.len << ") " << string(static_cast<char*>(v.bytes), v.len);
}

ostream& operator<<(ostream& os, const amqp_table_t& v) {
  os << "num_entries: " << v.num_entries;
  for(decltype(v.num_entries) i = 0; i < v.num_entries; ++i) {
    os << ' ' << v.entries[i];
  }
  return os;
}

ostream& operator<<(ostream& os, const amqp_field_value_t& v) {
  os << '(' << v.kind << ") ";
  switch(v.kind) {
    case AMQP_FIELD_KIND_DECIMAL:
      os << v.value.decimal.value << " decimals: " << v.value.decimal.decimals;
      break;
    case AMQP_FIELD_KIND_BYTES:
    case AMQP_FIELD_KIND_UTF8:
      os << v.value.bytes;
      break;
    case AMQP_FIELD_KIND_TABLE:
      os << v.value.table;
      break;
    case AMQP_FIELD_KIND_ARRAY:
      os << "num_entries: " << v.value.array.num_entries;
      for (decltype(v.value.array.num_entries) i = 0; i < v.value.array.num_entries; ++i) {
        os << ' ' << v.value.array.entries[i];
      }
      break;
    case AMQP_FIELD_KIND_BOOLEAN:
      os << v.value.boolean;
      break;
    case AMQP_FIELD_KIND_I8:
      os << (int)v.value.i8;
      break;
    case AMQP_FIELD_KIND_U8:
      os << (int)v.value.u8;
      break;
    case AMQP_FIELD_KIND_I16:
      os << (int)v.value.i16;
      break;
    case AMQP_FIELD_KIND_U16:
      os << (int)v.value.u16;
      break;
    case AMQP_FIELD_KIND_I32:
      os << v.value.i32;
      break;
    case AMQP_FIELD_KIND_U32:
      os << v.value.u32;
      break;
    case AMQP_FIELD_KIND_I64:
      os << v.value.i64;
      break;
    case AMQP_FIELD_KIND_U64:
      os << v.value.u64;
      break;
    case AMQP_FIELD_KIND_F32:
      os << v.value.f32;
      break;
    case AMQP_FIELD_KIND_F64:
      os << v.value.f64;
      break;
  }
  return os;
}

ostream& operator<<(ostream& os, const amqp_table_entry_t& v) {
  return os << "key: " << v.key << " value: " << v.value;
}

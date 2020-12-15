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

#include <gmock/gmock.h>

#include <amqp.h>

namespace rmqcxx { namespace unit_tests {

  extern const char* lastRPCMethod;

  class MockAMQP final {
  public:

    MockAMQP() : lastRPCMethod(nullptr) {
      current_ = this;
    }

    ~MockAMQP() noexcept = default;
    MOCK_METHOD4(basic_ack, int(amqp_connection_state_t, amqp_channel_t, uint64_t, amqp_boolean_t));
    MOCK_METHOD8(basic_consume, amqp_basic_consume_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_boolean_t, amqp_boolean_t, amqp_boolean_t, amqp_table_t));
    MOCK_METHOD5(basic_nack, int(amqp_connection_state_t, amqp_channel_t, uint64_t, amqp_boolean_t, amqp_boolean_t));
    MOCK_METHOD8(basic_publish, int(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_boolean_t, amqp_boolean_t, amqp_basic_properties_t const*, amqp_bytes_t));
    MOCK_METHOD5(basic_qos, void(amqp_connection_state_t, amqp_channel_t, uint32_t, uint16_t, amqp_boolean_t));
    MOCK_METHOD3(basic_recover, amqp_basic_recover_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_boolean_t));
    MOCK_METHOD3(channel_close, amqp_rpc_reply_t(amqp_connection_state_t, amqp_channel_t, int));
    MOCK_METHOD3(channel_flow, amqp_channel_flow_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_boolean_t));
    MOCK_METHOD2(channel_open, amqp_channel_open_ok_t*(amqp_connection_state_t, amqp_channel_t));
    MOCK_METHOD2(connection_close, amqp_rpc_reply_t(amqp_connection_state_t, int));
    MOCK_METHOD4(consume_message, amqp_rpc_reply_t(amqp_connection_state_t, amqp_envelope_t*, struct timeval*, int));
    MOCK_METHOD1(destroy_connection, int(amqp_connection_state_t));
    MOCK_METHOD1(destroy_envelope, void(amqp_envelope_t*));
    MOCK_METHOD1(destroy_message, void(amqp_message_t*));
    MOCK_METHOD1(error_string2, const char*(int));
    MOCK_METHOD6(exchange_bind, amqp_exchange_bind_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_bytes_t, amqp_table_t));
    MOCK_METHOD9(exchange_declare, amqp_exchange_declare_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_boolean_t, amqp_boolean_t, amqp_boolean_t, amqp_boolean_t, amqp_table_t));
    MOCK_METHOD4(exchange_delete, amqp_exchange_delete_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_boolean_t));
    MOCK_METHOD6(exchange_unbind, amqp_exchange_unbind_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_bytes_t, amqp_table_t));
    MOCK_METHOD2(get_rpc_reply, amqp_rpc_reply_t(amqp_connection_state_t, const std::string&)); // const std::string& is last rpc name
    MOCK_METHOD1(get_rpc_timeout, struct timeval*(amqp_connection_state_t));
    MOCK_METHOD7(login, amqp_rpc_reply_t(amqp_connection_state_t, char const*, int, int, int, amqp_sasl_method_enum, const std::vector<const char*>&));
    MOCK_METHOD8(login_with_properties, amqp_rpc_reply_t(amqp_connection_state_t, char const*, int, int, int, const amqp_table_t*, amqp_sasl_method_enum, const std::vector<const char*>&));
    MOCK_METHOD1(method_name, char const*(amqp_method_number_t));
    MOCK_METHOD0(new_connection, amqp_connection_state_t());
    MOCK_METHOD6(queue_bind, amqp_queue_bind_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_bytes_t, amqp_table_t));
    MOCK_METHOD8(queue_declare, amqp_queue_declare_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_boolean_t, amqp_boolean_t, amqp_boolean_t, amqp_boolean_t, amqp_table_t));
    MOCK_METHOD5(queue_delete, amqp_queue_delete_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_boolean_t, amqp_boolean_t));
    MOCK_METHOD3(queue_purge, amqp_queue_purge_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t));
    MOCK_METHOD6(queue_unbind, amqp_queue_unbind_ok_t*(amqp_connection_state_t, amqp_channel_t, amqp_bytes_t, amqp_bytes_t, amqp_bytes_t, amqp_table_t));
    MOCK_METHOD4(read_message, amqp_rpc_reply_t(amqp_connection_state_t, amqp_channel_t, amqp_message_t*, int));
    MOCK_METHOD2(set_handshake_timeout, int(amqp_connection_state_t, struct timeval*));
    MOCK_METHOD2(set_rpc_timeout, int(amqp_connection_state_t, struct timeval*));
    MOCK_METHOD3(simple_wait_frame_noblock, int(amqp_connection_state_t, amqp_frame_t*, struct timeval*));
    MOCK_METHOD4(socket_open_noblock, int(amqp_socket_t*, const char*, int, struct timeval*));
    MOCK_METHOD1(tcp_socket_new, amqp_socket_t*(amqp_connection_state_t));

    static MockAMQP* instance() noexcept {
      return current_;
    }
    const char* lastRPCMethod;
  private:
    static MockAMQP* current_;
  };

}} // namespace rmqcxx.unit_tests

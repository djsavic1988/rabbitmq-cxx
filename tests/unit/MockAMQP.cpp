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

#include <cstdarg>

#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>

#include "MockAMQP.hpp"

using rmqcxx::unit_tests::MockAMQP;

using std::string;
using std::vector;

MockAMQP* MockAMQP::current_ = nullptr;

int amqp_basic_ack(amqp_connection_state_t state, amqp_channel_t channel, uint64_t tag, amqp_boolean_t multiple) {
  MockAMQP::instance()->lastRPCMethod = "basic_ack";
  return MockAMQP::instance()->basic_ack(state, channel, tag, multiple);
}

amqp_basic_consume_ok_t* amqp_basic_consume(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue, amqp_bytes_t consumerTag, amqp_boolean_t noLocal, amqp_boolean_t noAck, amqp_boolean_t exclusive, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "basic_consume";
  return MockAMQP::instance()->basic_consume(state, channel, queue, consumerTag, noLocal, noAck, exclusive, arguments);
}

int amqp_basic_nack(amqp_connection_state_t state, amqp_channel_t channel, uint64_t deliveryTag, amqp_boolean_t multiple, amqp_boolean_t requeue) {
  MockAMQP::instance()->lastRPCMethod = "basic_nack";
  return MockAMQP::instance()->basic_nack(state, channel, deliveryTag, multiple, requeue);
}

int amqp_basic_publish(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t exchange, amqp_bytes_t routingKey, amqp_boolean_t mandatory, amqp_boolean_t immediate, amqp_basic_properties_t const *properties, amqp_bytes_t body) {
  MockAMQP::instance()->lastRPCMethod = "basic_publish";
  return MockAMQP::instance()->basic_publish(state, channel, exchange, routingKey, mandatory, immediate, properties, body);
}

amqp_basic_qos_ok_t* amqp_basic_qos(amqp_connection_state_t state, amqp_channel_t channel, uint32_t prefetchSize, uint16_t prefetchCount, amqp_boolean_t global) {
  static amqp_basic_qos_ok_t r{0};
  MockAMQP::instance()->lastRPCMethod = "basic_qos";
  MockAMQP::instance()->basic_qos(state, channel, prefetchSize, prefetchCount, global);
  return &r;
}

amqp_basic_recover_ok_t* amqp_basic_recover(amqp_connection_state_t state, amqp_channel_t channel, amqp_boolean_t requeue) {
  MockAMQP::instance()->lastRPCMethod = "basic_recover";
  return MockAMQP::instance()->basic_recover(state, channel, requeue);
}

amqp_rpc_reply_t amqp_channel_close(amqp_connection_state_t state, amqp_channel_t channelId, int code) {
  MockAMQP::instance()->lastRPCMethod = "channel_close";
  return MockAMQP::instance()->channel_close(state, channelId, code);
}

amqp_channel_flow_ok_t* amqp_channel_flow(amqp_connection_state_t state, amqp_channel_t channel, amqp_boolean_t active) {
  MockAMQP::instance()->lastRPCMethod = "channel_flow";
  return MockAMQP::instance()->channel_flow(state, channel, active);
}

amqp_channel_open_ok_t* amqp_channel_open(amqp_connection_state_t state, amqp_channel_t channelId) {
  MockAMQP::instance()->lastRPCMethod = "channel_open";
  return MockAMQP::instance()->channel_open(state, channelId);
}

amqp_rpc_reply_t amqp_connection_close(amqp_connection_state_t state, int code) {
  MockAMQP::instance()->lastRPCMethod = "connection_close";
  return MockAMQP::instance()->connection_close(state, code);
}

amqp_rpc_reply_t amqp_consume_message(amqp_connection_state_t state, amqp_envelope_t* envelope, struct timeval* timeout, int flags) {
  return MockAMQP::instance()->consume_message(state, envelope, timeout, flags);
}

int amqp_destroy_connection(amqp_connection_state_t state) {
  return MockAMQP::instance()->destroy_connection(state);
}

void amqp_destroy_envelope(amqp_envelope_t* envelope) {
  MockAMQP::instance()->destroy_envelope(envelope);
}

void amqp_destroy_message(amqp_message_t* message) {
  MockAMQP::instance()->destroy_message(message);
}

const char *amqp_error_string2(int code) {
  return MockAMQP::instance()->error_string2(code);
}

amqp_exchange_bind_ok_t* amqp_exchange_bind(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t destination, amqp_bytes_t source, amqp_bytes_t routingKey, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "exchange_bind";
  return MockAMQP::instance()->exchange_bind(state, channel, destination, source, routingKey, arguments);
}

amqp_exchange_declare_ok_t* amqp_exchange_declare(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t exchange, amqp_bytes_t type, amqp_boolean_t passive, amqp_boolean_t durable, amqp_boolean_t autoDelete, amqp_boolean_t internal, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "exchange_declare";
  return MockAMQP::instance()->exchange_declare(state, channel, exchange, type, passive, durable, autoDelete, internal, arguments);
}

amqp_exchange_delete_ok_t* amqp_exchange_delete(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t exchange, amqp_boolean_t ifUnused) {
  MockAMQP::instance()->lastRPCMethod = "exchange_delete";
  return MockAMQP::instance()->exchange_delete(state, channel, exchange, ifUnused);
}

amqp_exchange_unbind_ok_t* amqp_exchange_unbind(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t destination, amqp_bytes_t source, amqp_bytes_t routingKey, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "exchange_unbind";
  return MockAMQP::instance()->exchange_unbind(state, channel, destination, source, routingKey, arguments);
}

amqp_rpc_reply_t amqp_get_rpc_reply(amqp_connection_state_t state) {
  auto tmp = MockAMQP::instance()->lastRPCMethod;
  MockAMQP::instance()->lastRPCMethod = nullptr;
  return MockAMQP::instance()->get_rpc_reply(state, tmp);
}

struct timeval* amqp_get_rpc_timeout(amqp_connection_state_t state) {
  return MockAMQP::instance()->get_rpc_timeout(state);
}

amqp_rpc_reply_t amqp_login(amqp_connection_state_t state, char const* vhost, int channelMax, int frameMax, int heartbeat, amqp_sasl_method_enum saslMethod, ...) {
  vector<const char*> arguments;
  va_list vl;
  va_start(vl, saslMethod);
  switch(saslMethod) {
    case AMQP_SASL_METHOD_PLAIN:
      arguments.emplace_back(va_arg(vl, const char*));
      // intentionally no break
    case AMQP_SASL_METHOD_EXTERNAL:
      arguments.emplace_back(va_arg(vl, const char*));
      break;
    default:
      break;
  }
  va_end(vl);
  return MockAMQP::instance()->login(state, vhost, channelMax, frameMax, heartbeat, saslMethod, arguments);
}

amqp_rpc_reply_t amqp_login_with_properties(amqp_connection_state_t state, char const* vhost, int channelMax, int frameMax, int heartbeat, const amqp_table_t* clientProperties, amqp_sasl_method_enum saslMethod, ...) {
  vector<const char*> arguments;
  va_list vl;
  va_start(vl, saslMethod);
  switch(saslMethod) {
    case AMQP_SASL_METHOD_PLAIN:
      arguments.emplace_back(va_arg(vl, const char*));
      // intentionally no break
    case AMQP_SASL_METHOD_EXTERNAL:
      arguments.emplace_back(va_arg(vl, const char*));
      break;
    default:
      break;
  }
  va_end(vl);
  return MockAMQP::instance()->login_with_properties(state, vhost, channelMax, frameMax, heartbeat, clientProperties, saslMethod, arguments);
}

char const* amqp_method_name(amqp_method_number_t methodNumber) {
  return MockAMQP::instance()->method_name(methodNumber);
}

amqp_connection_state_t amqp_new_connection() {
  return MockAMQP::instance()->new_connection();
}

amqp_queue_bind_ok_t* amqp_queue_bind(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue, amqp_bytes_t exchange, amqp_bytes_t routingKey, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "queue_bind";
  return MockAMQP::instance()->queue_bind(state, channel, queue, exchange, routingKey, arguments);
}

amqp_queue_declare_ok_t* amqp_queue_declare(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue, amqp_boolean_t passive, amqp_boolean_t durable, amqp_boolean_t exclusive, amqp_boolean_t autoDelete, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "queue_declare";
  return MockAMQP::instance()->queue_declare(state, channel, queue, passive, durable, exclusive, autoDelete, arguments);
}

amqp_queue_delete_ok_t* amqp_queue_delete(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue, amqp_boolean_t ifUnused, amqp_boolean_t ifEmpty) {
  MockAMQP::instance()->lastRPCMethod = "queue_delete";
  return MockAMQP::instance()->queue_delete(state, channel, queue, ifUnused, ifEmpty);
}

amqp_queue_purge_ok_t* amqp_queue_purge(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue) {
  MockAMQP::instance()->lastRPCMethod = "queue_purge";
  return MockAMQP::instance()->queue_purge(state, channel, queue);
}

amqp_queue_unbind_ok_t* amqp_queue_unbind(amqp_connection_state_t state, amqp_channel_t channel, amqp_bytes_t queue, amqp_bytes_t exchange, amqp_bytes_t routingKey, amqp_table_t arguments) {
  MockAMQP::instance()->lastRPCMethod = "queue_unbind";
  return MockAMQP::instance()->queue_unbind(state, channel, queue, exchange, routingKey, arguments);
}

amqp_rpc_reply_t amqp_read_message(amqp_connection_state_t state, amqp_channel_t channel, amqp_message_t* message, int flags) {
  return MockAMQP::instance()->read_message(state, channel, message, flags);
}

int amqp_set_handshake_timeout(amqp_connection_state_t state, struct timeval* timeout) {
  return MockAMQP::instance()->set_handshake_timeout(state, timeout);
}

int amqp_set_rpc_timeout(amqp_connection_state_t state, struct timeval* tv) {
  return MockAMQP::instance()->set_rpc_timeout(state, tv);
}

int amqp_simple_wait_frame_noblock(amqp_connection_state_t state, amqp_frame_t* decodedFrame, struct timeval* timeout) {
  return MockAMQP::instance()->simple_wait_frame_noblock(state, decodedFrame, timeout);
}

int amqp_socket_open_noblock(amqp_socket_t* self, const char* host, int port, struct timeval* timeout) {
  return MockAMQP::instance()->socket_open_noblock(self, host, port, timeout);
}

amqp_socket_t* amqp_tcp_socket_new(amqp_connection_state_t state) {
  return MockAMQP::instance()->tcp_socket_new(state);
}

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

#include "ConnectionTest.hpp"

namespace rmqcxx { namespace unit_tests {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::Test;

using std::chrono::seconds;
using std::forward;
using std::string;
using std::vector;

struct ConnectionLoginTest : public ConnectionTest {
  template <typename ExceptionType, typename... Args>
  void loginFailureTest(bool withProperties, amqp_rpc_reply_t loginReply, Args... args) {
    vector<const char*> loginArguments {{forward<Args>(args)...}};
    EXPECT_CALL(amqp, new_connection())
      .WillOnce(Return(connPtr));

    EXPECT_CALL(amqp, tcp_socket_new(connPtr))
      .WillOnce(Return(socketPtr));

    EXPECT_CALL(amqp, socket_open_noblock(socketPtr, address.c_str(), port, Pointee(connectTv)))
      .WillOnce(Return(0));

    if (withProperties) {
      EXPECT_CALL(amqp, login_with_properties(connPtr, vhost.c_str(), maxChannels, maxFrameSize, heartbeat, &properties, saslMethod, loginArguments))
        .WillOnce(::testing::Return(loginReply));
    } else {
      EXPECT_CALL(amqp, login(connPtr, vhost.c_str(), maxChannels, maxFrameSize, heartbeat, saslMethod, loginArguments))
        .WillOnce(::testing::Return(loginReply));
    }

    EXPECT_CALL(amqp, connection_close(connPtr, AMQP_REPLY_SUCCESS));

    EXPECT_CALL(amqp, get_rpc_reply(connPtr, "connection_close"))
      .WillOnce(Return(amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_NORMAL }));

    EXPECT_CALL(amqp, destroy_connection(connPtr));

    EXPECT_THROW(Connection connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, static_cast<const seconds*>(nullptr), withProperties ? &properties : nullptr, saslMethod, forward<Args>(args)...), ExceptionType);
  }
};

TEST_F(ConnectionTest, FailureToAllocateConnection) {

  EXPECT_CALL(amqp, new_connection())
    .WillOnce(Return(nullptr));

  EXPECT_THROW(Connection connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, static_cast<const seconds*>(nullptr), nullptr, saslMethod), Exception);
}

TEST_F(ConnectionTest, FailureToAllocateSocket) {

  EXPECT_CALL(amqp, new_connection())
    .WillOnce(Return(connPtr));

  EXPECT_CALL(amqp, tcp_socket_new(connPtr))
    .WillOnce(Return(nullptr));

  EXPECT_CALL(amqp, destroy_connection(connPtr));

  try {
    Connection connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, static_cast<const seconds*>(nullptr), nullptr, saslMethod);
    ASSERT_TRUE(false);
  } catch(const SocketException& ex) {
    EXPECT_EQ(ex.socket, nullptr);
    EXPECT_EQ(ex.status, AMQP_STATUS_SOCKET_ERROR);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionTest, FailureToOpenSocket) {

  EXPECT_CALL(amqp, new_connection())
    .WillOnce(Return(connPtr));

  EXPECT_CALL(amqp, tcp_socket_new(connPtr))
    .WillOnce(Return(socketPtr));

  EXPECT_CALL(amqp, socket_open_noblock(socketPtr, address.c_str(), port, Pointee(connectTv)))
    .WillOnce(Return(-1));

  EXPECT_CALL(amqp, connection_close(connPtr, AMQP_REPLY_SUCCESS));

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "connection_close"))
    .WillOnce(Return(amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_NORMAL }));

  EXPECT_CALL(amqp, destroy_connection(connPtr));

  try {
    Connection connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, static_cast<const seconds*>(nullptr), nullptr, saslMethod);
    ASSERT_TRUE(false);
  } catch(const SocketException& ex) {
    EXPECT_EQ(ex.socket, socketPtr);
    EXPECT_EQ(ex.status, -1);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionTest, FailureToSetHandshakeTimeout) {

  EXPECT_CALL(amqp, new_connection())
    .WillOnce(Return(connPtr));

  EXPECT_CALL(amqp, tcp_socket_new(connPtr))
    .WillOnce(Return(socketPtr));

  EXPECT_CALL(amqp, socket_open_noblock(socketPtr, address.c_str(), port, Pointee(connectTv)))
    .WillOnce(Return(0));

  EXPECT_CALL(amqp, set_handshake_timeout(connPtr, Pointee(handshakeTv)))
    .WillOnce(Return(-2));

  EXPECT_CALL(amqp, connection_close(connPtr, AMQP_REPLY_SUCCESS));

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "connection_close"))
    .WillOnce(Return(amqp_rpc_reply_t {.reply_type = AMQP_RESPONSE_NORMAL }));

  EXPECT_CALL(amqp, destroy_connection(connPtr));

  try {
    Connection connection(address, port, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout, &handshakeTimeout, nullptr, saslMethod);
    ASSERT_TRUE(false);
  } catch(const OperationException& ex) {
    EXPECT_EQ(ex.status, -2);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionLoginTest, FailureToLogin) {
  saslMethod = AMQP_SASL_METHOD_PLAIN;
  EXPECT_CALL(amqp, method_name(42))
    .WillOnce(Return("unit test method"));

  loginFailureTest<RPCException>(true, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_SERVER_EXCEPTION, .reply = amqp_method_t { .id = 42 }}, "username", "password");

  EXPECT_CALL(amqp, error_string2(43))
    .WillOnce(Return("unit test error"));

  loginFailureTest<LibraryException>(true, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = 43 }, "username", "password");

  EXPECT_CALL(amqp, method_name(42))
    .WillOnce(Return("unit test method"));

  loginFailureTest<RPCException>(false, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_SERVER_EXCEPTION, .reply = amqp_method_t { .id = 42 }}, "username", "password");

  EXPECT_CALL(amqp, error_string2(43))
    .WillOnce(Return("unit test error"));

  loginFailureTest<LibraryException>(false, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = 43 }, "username", "password");

  saslMethod = AMQP_SASL_METHOD_EXTERNAL;
  EXPECT_CALL(amqp, method_name(42))
    .WillOnce(Return("unit test method"));

  loginFailureTest<RPCException>(true, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_SERVER_EXCEPTION, .reply = amqp_method_t { .id = 42 }}, "external");

  EXPECT_CALL(amqp, error_string2(43))
    .WillOnce(Return("unit test error"));

  loginFailureTest<LibraryException>(true, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = 43 }, "external");

  EXPECT_CALL(amqp, method_name(42))
    .WillOnce(Return("unit test method"));

  loginFailureTest<RPCException>(false, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_SERVER_EXCEPTION, .reply = amqp_method_t { .id = 42 }}, "external");

  EXPECT_CALL(amqp, error_string2(43))
    .WillOnce(Return("unit test error"));

  loginFailureTest<LibraryException>(false, amqp_rpc_reply_t { .reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = 43 }, "external");
}

TEST_F(ConnectionTest, ConnectionCreation) {
  std::string username("username"), password("password"), external("external");

  saslMethod = AMQP_SASL_METHOD_PLAIN;
  createConnection(false, false, "username", "password");
  createConnection(false, true, "username", "password");
  createConnection(true, false, "username", "password");
  createConnection(true, true, "username", "password");
  prepareConnectionCreation(false, false, username.c_str(), password.c_str());
  Connection {address, port, username, password, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout };


  saslMethod = AMQP_SASL_METHOD_EXTERNAL;
  createConnection(false, false, "external");
  createConnection(false, true, "external");
  createConnection(true, false, "external");
  createConnection(true, true, "external");
  prepareConnectionCreation(false, false, external.c_str());
  Connection {address, port, external, vhost, maxChannels, maxFrameSize, heartbeat, connectTimeout };
}

TEST_F(ConnectionTest, ConsumeEnvelope) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_envelope_t* envelope(nullptr);
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(DoAll(SaveArg<1>(&envelope), SetArgPointee<1>(amqp_envelope_t{.channel = 1}),
      Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_NORMAL })));
  bool called = false;
  EXPECT_TRUE(conn.consumeEnvelope(consumeTimeout, [this, &called] (Envelope env) {
    EXPECT_CALL(amqp, destroy_envelope(const_cast<amqp_envelope_t*>(static_cast<const amqp_envelope_t*>(env))));
    called = true;
  }));
  EXPECT_TRUE(called);
}

TEST_F(ConnectionTest, ConsumeEnvelopeWaitForever) {
  auto conn = createSimpleConnection();

  amqp_envelope_t* envelope(nullptr);
  EXPECT_CALL(amqp, consume_message(connPtr, _, nullptr, 0))
    .WillOnce(DoAll(SaveArg<1>(&envelope), SetArgPointee<1>(amqp_envelope_t{.channel = 1}),
      Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_NORMAL })));

  bool called = false;
  conn.consumeEnvelope([this, &called] (Envelope env) {
    EXPECT_CALL(amqp, destroy_envelope(const_cast<amqp_envelope_t*>(static_cast<const amqp_envelope_t*>(env))));
    called = true;
  });
  EXPECT_TRUE(called);

}

TEST_F(ConnectionTest, ConsumeEmptyEnvelopeTimeout) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_envelope_t{.channel = 0}),
      Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_NORMAL })));
  EXPECT_FALSE(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}));
}

TEST_F(ConnectionTest, ConsumeUnhandledRPCReply) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_SERVER_EXCEPTION }));

  EXPECT_THROW(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}), RPCException);
}

TEST_F(ConnectionTest, ConsumeUnhandledLibraryError) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_INVALID_PARAMETER }));

  EXPECT_THROW(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}), RPCException);
}

TEST_F(ConnectionTest, ConsumeLibrarySocketError) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_SOCKET_ERROR }));

  EXPECT_THROW(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}), SocketException);
}

TEST_F(ConnectionTest, ConsumeLibraryTimeout) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_envelope_t{.channel = 0}),
      Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_TIMEOUT })));
  EXPECT_FALSE(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}));
}

TEST_F(ConnectionTest, ConsumeFrameTimeout) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_envelope_t{.channel = 0}),
      Return(amqp_rpc_reply_t{.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE })));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(Return(AMQP_STATUS_TIMEOUT));
  EXPECT_FALSE(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}));
}

TEST_F(ConnectionTest, ConsumeUnhandledFrameStatus) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(Return(AMQP_STATUS_NO_MEMORY));
  try {
    conn.consumeEnvelope(consumeTimeout, [] (Envelope) {});
    ASSERT_TRUE(false);
  } catch(const FrameStatusException& ex) {
    EXPECT_EQ(ex.status, AMQP_STATUS_NO_MEMORY);
    EXPECT_EQ(ex.reply, reply);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionTest, ConsumeUnhandledFrameType) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t {.frame_type = 0 }),Return(AMQP_STATUS_OK)));
  try {
    conn.consumeEnvelope(consumeTimeout, [] (Envelope) {});
    ASSERT_TRUE(false);
  } catch(const FrameException& ex) {
    EXPECT_EQ(ex.frame, amqp_frame_t {.frame_type = 0 });
    EXPECT_EQ(ex.reply, reply);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionTest, ConsumeAckReceived) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  amqp_basic_ack_t basicAck { .delivery_tag = 99UL, .multiple = false };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t {.frame_type = AMQP_FRAME_METHOD, .payload = { amqp_method_t{.id = AMQP_BASIC_ACK_METHOD, .decoded = &basicAck}}}),Return(AMQP_STATUS_OK)));

  bool called = false;
  EXPECT_TRUE(conn.consume(consumeTimeout, [] (Envelope) {}, [] (ReturnedMessage) {}, [&called, &basicAck] (amqp_basic_ack_t ackMethod) {
    EXPECT_EQ(memcmp(&ackMethod, &basicAck, sizeof(basicAck)), 0);
    called = true;
  }));
  EXPECT_TRUE(called);
}

TEST_F(ConnectionTest, ConsumeMessageReturn) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  amqp_basic_return_t basicReturn {
    .reply_code = 0,
    .reply_text = amqp_bytes_t {.len = ::strlen("replyText"), .bytes = const_cast<char*>("replyText")},
    .exchange = amqp_bytes_t {.len = ::strlen("exchange"), .bytes = const_cast<char*>("exchange")},
    .routing_key = amqp_bytes_t {.len = ::strlen("routingKey"), .bytes = const_cast<char*>("routingKey")},
  };

  amqp_message_t message {
  };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t { .frame_type = AMQP_FRAME_METHOD, .channel = 15, .payload = { amqp_method_t{.id = AMQP_BASIC_RETURN_METHOD, .decoded = &basicReturn}}}),Return(AMQP_STATUS_OK)));

  EXPECT_CALL(amqp, read_message(connPtr, 15, _, 0))
    .WillOnce(DoAll(SetArgPointee<2>(message), Return(amqp_rpc_reply_t{ .reply_type = AMQP_RESPONSE_NORMAL})));

  bool called = false;
  EXPECT_TRUE(conn.consumeReturnedMessage(consumeTimeout, [this, &called, &basicReturn, &message] (ReturnedMessage returnMethod) {
    EXPECT_EQ(returnMethod.method(), basicReturn);
    EXPECT_EQ(returnMethod.message(), message);
    EXPECT_CALL(amqp, destroy_message(const_cast<amqp_message_t*>(static_cast<const amqp_message_t*>(returnMethod.message()))));
    called = true;
  }));
  EXPECT_TRUE(called);
}

TEST_F(ConnectionTest, ConsumeFailedMessageReturn) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  amqp_basic_return_t basicReturn {
    .reply_code = 0,
    .reply_text = amqp_bytes_t {.len = ::strlen("replyText"), .bytes = const_cast<char*>("replyText")},
    .exchange = amqp_bytes_t {.len = ::strlen("exchange"), .bytes = const_cast<char*>("exchange")},
    .routing_key = amqp_bytes_t {.len = ::strlen("routingKey"), .bytes = const_cast<char*>("routingKey")},
  };

  amqp_message_t message {
  };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t { .frame_type = AMQP_FRAME_METHOD, .channel = 15, .payload = { amqp_method_t{.id = AMQP_BASIC_RETURN_METHOD, .decoded = &basicReturn}}}),Return(AMQP_STATUS_OK)));

  EXPECT_CALL(amqp, read_message(connPtr, 15, _, 0))
    .WillOnce(DoAll(SetArgPointee<2>(message), Return(amqp_rpc_reply_t{ .reply_type = AMQP_RESPONSE_SERVER_EXCEPTION})));

  EXPECT_CALL(amqp, method_name(_))
    .WillOnce(Return("unit test method name"));
  EXPECT_CALL(amqp, destroy_message(_));
  EXPECT_THROW(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}), RPCException);
}

TEST_F(ConnectionTest, ConsumeChannelCloseReceived) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  amqp_channel_close_t channelClose { };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t {.frame_type = AMQP_FRAME_METHOD, .channel = 11, .payload = { amqp_method_t{.id = AMQP_CHANNEL_CLOSE_METHOD, .decoded = &channelClose}}}),Return(AMQP_STATUS_OK)));

  try {
    conn.consumeEnvelope(consumeTimeout, [] (Envelope) {});
    ASSERT_TRUE(false);
  } catch(const ChannelCloseException& ex) {
    EXPECT_EQ(ex.channel, 11);
  } catch(...) {
    ASSERT_TRUE(false);
  }
}

TEST_F(ConnectionTest, ConsumeConnectionCloseReceived) {
  auto conn = createSimpleConnection();

  seconds consumeTimeout(55);
  struct timeval consumeTv{.tv_sec = consumeTimeout.count(), .tv_usec = 0};

  amqp_rpc_reply_t reply {.reply_type = AMQP_RESPONSE_LIBRARY_EXCEPTION, .library_error = AMQP_STATUS_UNEXPECTED_STATE };

  amqp_connection_close_t connectionClose { };

  EXPECT_CALL(amqp, destroy_envelope(_)); // destroys the empty envelope
  EXPECT_CALL(amqp, consume_message(connPtr, _, Pointee(consumeTv), 0))
    .WillOnce(Return(reply));
  EXPECT_CALL(amqp, simple_wait_frame_noblock(connPtr, _, Pointee(consumeTv)))
    .WillOnce(DoAll(SetArgPointee<1>(amqp_frame_t {.frame_type = AMQP_FRAME_METHOD, .channel = 11, .payload = { amqp_method_t{.id = AMQP_CONNECTION_CLOSE_METHOD, .decoded = &connectionClose}}}),Return(AMQP_STATUS_OK)));

  EXPECT_THROW(conn.consumeEnvelope(consumeTimeout, [] (Envelope) {}), ConnectionCloseException);
}

TEST_F(ConnectionTest, SetRPCTimeout) {
  auto conn = createSimpleConnection();
  seconds rpcTimeout(1);
  struct timeval rpcTv { .tv_sec = rpcTimeout.count(), .tv_usec = 0};
  EXPECT_CALL(amqp, set_rpc_timeout(connPtr, Pointee(rpcTv)))
    .WillOnce(Return(AMQP_STATUS_OK));
  conn.setRpcTimeout(rpcTimeout);
}

TEST_F(ConnectionTest, FailureToSetRPCTimeout) {
  auto conn = createSimpleConnection();
  seconds rpcTimeout(1);
  struct timeval rpcTv { .tv_sec = rpcTimeout.count(), .tv_usec = 0};
  EXPECT_CALL(amqp, set_rpc_timeout(connPtr, Pointee(rpcTv)))
    .WillOnce(Return(AMQP_STATUS_SOCKET_ERROR));
  EXPECT_THROW(conn.setRpcTimeout(rpcTimeout), ConnectionException);
}

TEST_F(ConnectionTest, GetRPCTimeout) {
  auto conn = createSimpleConnection();
  seconds rpcTimeout(0);
  struct timeval tv { .tv_sec = 1, .tv_usec = 0};
  EXPECT_CALL(amqp, get_rpc_timeout)
    .WillOnce(Return(&tv));
  EXPECT_TRUE(conn.getRpcTimeout(rpcTimeout));
  EXPECT_EQ(rpcTimeout, seconds(1));
}

TEST_F(ConnectionTest, FailedGetRPCTimeout) {
  auto conn = createSimpleConnection();
  seconds rpcTimeout(0);
  EXPECT_CALL(amqp, get_rpc_timeout)
    .WillOnce(Return(nullptr));
  EXPECT_FALSE(conn.getRpcTimeout(rpcTimeout));
  EXPECT_EQ(rpcTimeout, seconds(0));
}

TEST_F(ConnectionTest, GenericRPC) {
  auto conn = createSimpleConnection();

  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc without args"))
    .WillOnce(Return(normalReply));
  bool called = false;
  conn.rpc([&called, this] (::amqp_connection_state_t state) {
    MockAMQP::instance()->lastRPCMethod = "rpc without args";
    EXPECT_EQ(state, connPtr);
    called = true;
  });
  EXPECT_TRUE(called);

  // rpc with args
  EXPECT_CALL(amqp, get_rpc_reply(connPtr, "rpc with args"))
    .WillOnce(Return(normalReply));
  called = false;
  conn.rpc([&called, this] (::amqp_connection_state_t state, int arg) {
    MockAMQP::instance()->lastRPCMethod = "rpc with args";
    EXPECT_EQ(state, connPtr);
    EXPECT_EQ(arg, 99);
    called = true;
  }, 99);
  EXPECT_TRUE(called);
}

}} // namespace rmqcxx.unit_tests

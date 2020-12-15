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

#include <sys/time.h>

#include <amqp.h>

#include <string>

bool operator==(const struct timeval& l, const struct timeval& r);

bool operator==(const amqp_bytes_t& l, const amqp_bytes_t& r);

bool operator==(const amqp_method_t& l, const amqp_method_t& r);

bool operator==(const amqp_rpc_reply_t& l, const amqp_rpc_reply_t& r);

bool operator==(const amqp_basic_return_t& l, const amqp_basic_return_t& r);

bool operator==(const amqp_frame_t& l, const amqp_frame_t& r);

bool operator==(const amqp_table_t& l, const amqp_table_t& r);

bool operator==(const amqp_field_value_t& l, const amqp_field_value_t& r);

bool operator==(const amqp_table_entry_t& l, const amqp_table_entry_t& r);

bool operator==(const amqp_table_t& l, const amqp_table_t& r);

bool operator==(const amqp_basic_properties_t& l, const amqp_basic_properties_t& r);

bool operator==(const amqp_message_t& l, const amqp_message_t& r);

std::ostream& operator<<(std::ostream& os, const struct timeval& v);

std::ostream& operator<<(std::ostream& os, const amqp_bytes_t& v);

std::ostream& operator<<(std::ostream& os, const amqp_table_t& v);

std::ostream& operator<<(std::ostream& os, const amqp_field_value_t& v);

std::ostream& operator<<(std::ostream& os, const amqp_table_entry_t& v);

std::ostream& operator<<(std::ostream& os, const amqp_table_t& v);

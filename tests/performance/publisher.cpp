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

#include <chrono>

#include <benchmark/benchmark.h>

#include <rmqcxx.hpp>

using namespace benchmark;
using namespace rmqcxx;
using namespace std;
using namespace std::chrono;

static void simpleDirectPublisher(State& state) {
  Connection connection("172.17.0.2", 5672, "guest", "guest", "/", 0, 131072, 1, seconds(1));
  Channel channel(connection,1);
  Queue queue(channel, "queue0");

  queue.declare(false, false, true, true);

  const size_t kEnvelopes(10000);

  queue.consume("", false, false, true);

  for (auto _ : state) {
    for (size_t i=0; i < kEnvelopes; ++i)
      channel.publish("", "queue0", false, false, "{}");
    state.PauseTiming();
    for (size_t i=0; i < kEnvelopes; ++i)
      connection.consumeEnvelope([&channel] (const Envelope& envelope) { channel.ack(envelope->delivery_tag, false); });
    state.ResumeTiming();
  }
}
BENCHMARK(simpleDirectPublisher);
BENCHMARK_MAIN();


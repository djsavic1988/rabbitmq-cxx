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

#include <iostream>

#include <rmqcxx.hpp>

using namespace rmqcxx;

using namespace std;
using namespace std::chrono;

int main() {
  Connection connection("172.17.0.2", 5672, "guest", "guest", "consumer-test", 0, 131072, 1, seconds(1));
  Channel channel(connection,1);
  Queue queue(channel, "queue0");
  Exchange exchange(channel, "exchange0");

  exchange.declare("topic", false, false, true);
  cout << "declared exchange" << endl;

  TableEntry arg("x-queue-type", "classic");
  auto declaredQueue = queue.declare(false, true, false, false, arg);
  cout << "Declared queue: " << container<string>(declaredQueue->queue)
    << " message count: " << declaredQueue->message_count
    << " consumer count"<< declaredQueue->consumer_count << endl;

  cout << queue.consume("", false, false, false) << endl;

  queue.bind("exchange0", "#");

  do {
    try {
      if (!connection.consumeEnvelope(seconds(1), [&channel] (Envelope envelope) {
          cout << "Received: " << envelope.body() << endl;
          if (0 != channel.ack(envelope->delivery_tag, false)) {
            cout << "Failed to acknowledge envelope!" << endl;}})) {
        cout << "timeout" << endl;
      }
   } catch(const rmqcxx::Exception& ex) {
      cerr << "Failed: " << ex.what() << endl;
      break;
    }
  } while (true);
}

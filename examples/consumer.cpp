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
      if (!connection.consumeEnvelope(std::chrono::seconds(1), [&channel] (Envelope envelope) {
          cout << "Received: " << envelope.body() << endl;
          if (0 != channel.ack(envelope->delivery_tag, false)) {
            cout << "Failed to acknowledge envelope!" << endl;}})) {
        cout << "timeout" << endl;
      }
   } catch(rmqcxx::Exception& ex) {
      cerr << "Failed: " << ex.what() << endl;
      break;
    }
  } while (true);
}

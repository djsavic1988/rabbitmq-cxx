#include <iostream>
#include <rmqcxx.hpp>

using namespace std;
using namespace std::chrono;
using namespace rmqcxx;

int main() {

  Connection connection("172.17.0.2", 5672, "guest", "guest", "/", 0, 131072, 1, seconds(1));
  Channel channel(connection, 1);
  Exchange exchange(channel, "publish_exchange");
  exchange.declare("topic", false, false, false);

  channel.publish(exchange.name(), "nokey", true, false, "body");

  do {
    try {
      if (!connection.consumeReturnedMessage(std::chrono::seconds(1), [] (ReturnedMessage returnedMessage) {
        cout << "returned: " << container<string>(returnedMessage.message()->body)
            << " code: " << returnedMessage.method().reply_code
            << " reply_text: " << container<string>(returnedMessage.method().reply_text)
            << " exchange: " << container<string>(returnedMessage.method().exchange)
            << " routing_key: " << container<string>(returnedMessage.method().routing_key)
            << endl;})) {
        cout << "consume timeout" << endl;
      }
   } catch(rmqcxx::Exception& ex) {
      cerr << "Failed: " << ex.what() << endl;
      break;
    }
  } while (true);

  return 0;

}

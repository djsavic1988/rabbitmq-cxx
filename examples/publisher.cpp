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
      const auto& result = connection.consume(std::chrono::seconds(1));
      switch(result.index()) {
        case 1: {
          const auto& envelope = std::get<1>(result);
          cout << "Received: " << envelope.body() << endl;
          if (0 != channel.ack(envelope->delivery_tag, false)) {
            cout << "Failed to acknowledge envelope!" << endl;
          }
        }
          break;
        case 2: {
          const auto& returnedMessage = std::get<2>(result);
          cout << "returned: " << container<string>(returnedMessage.message()->body)
            << " code: " << returnedMessage.method().reply_code
            << " reply_text: " << container<string>(returnedMessage.method().reply_text)
            << " exchange: " << container<string>(returnedMessage.method().exchange)
            << " routing_key: " << container<string>(returnedMessage.method().routing_key)
            << endl;
        }
          break;
        case 0:
          cout << "timeout" << endl;
          break;
        default:
          break;
      }
   } catch(rmqcxx::Exception& ex) {
      cerr << "Failed: " << ex.what() << endl;
      break;
    }
  } while (true);

  return 0;

}

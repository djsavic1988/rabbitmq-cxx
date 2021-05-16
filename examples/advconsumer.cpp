
// Header start
#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <rmqcxx.hpp>

class Consumer {
public:
  struct Message {
    uint64_t id;
    std::string body;
  };

  template <typename QueuesContainer>
  Consumer(std::string host, int port, std::string vhost, std::string username, std::string password, QueuesContainer queuesToConsume) noexcept :
    host_(std::move(host)), port_(port), vhost_(std::move(vhost)), username_(std::move(username)), password_(std::move(password)), queuesToConsume_(std::begin(queuesToConsume), std::end(queuesToConsume)), run_(false) {
  }

  Consumer(std::string host, int port, std::string vhost, std::string username, std::string password, std::vector<std::string> queuesToConsume) noexcept :
    host_(std::move(host)), port_(port), vhost_(std::move(vhost)), username_(std::move(username)), password_(std::move(password)), queuesToConsume_(std::move(queuesToConsume)), run_(false) {
  }

  ~Consumer() noexcept;
  Consumer(const Consumer&) = delete;
  Consumer(Consumer&&) noexcept = delete;
  Consumer& operator=(const Consumer&) = delete;
  Consumer& operator=(Consumer&&) noexcept = delete;

  bool receive(Message& dst);
  void acknowledge(uint64_t id);

  void start();
  void stop() noexcept;
  void join() noexcept;

protected:

private:

  // Connection information
  std::string host_;
  int port_;
  std::string vhost_;
  std::string username_;
  std::string password_;
  std::vector<std::string> queuesToConsume_;

  /// Worker information
  std::thread worker_;
  std::atomic<bool> run_;

  // Message queues
  std::queue<Message> messages_;
  std::mutex messageMtx_;
  std::queue<uint64_t> ackQueue_;
  std::mutex ackMtx_;
};

// Header end

// Source start
#include <chrono>
#include <iostream>

using std::cerr;
using std::chrono::seconds;
using std::cout;
using std::endl;
using std::move;
using std::string;
using std::thread;
using std::vector;

using rmqcxx::Connection;
using rmqcxx::Channel;
using rmqcxx::Envelope;
using rmqcxx::Exception;
using rmqcxx::Queue;
using rmqcxx::TableEntry;


void Consumer::start() {
  run_ = true;
  worker_ = thread([this] () {
    while(run_) {
      try {
        Connection connection(host_, port_, username_, password_, vhost_, 0, 131072, 1, seconds(1));
        Channel channel(connection, 1);
        vector<Queue> queues;
        queues.reserve(queuesToConsume_.size());
        TableEntry arg("x-queue-type", "classic");
        for (const auto& x : queuesToConsume_) {
          queues.emplace_back(channel, x);
          queues.back().consume(string(), false, false, false, arg);
        }
        while (run_) {
          connection.consumeEnvelope(seconds(0), [this] (Envelope envelope) {
            messages_.emplace(Message{envelope->delivery_tag, envelope.body()});
          });
          {
            std::lock_guard<decltype(messageMtx_)> lk(ackMtx_);
            if (!ackQueue_.empty()) {
              channel.ack(ackQueue_.front(), false);
              ackQueue_.pop();
            }
          }
        }
      } catch(const Exception& ex) {
        // will reconnect on any exception, could be done so it reconnects only on the required exceptions
        cerr << ex.what() << endl;
        {
          std::lock_guard<decltype(messageMtx_)> lk(messageMtx_);
          messages_ = decltype(messages_)();
        }
        {
          std::lock_guard<decltype(messageMtx_)> lk(ackMtx_);
          ackQueue_ = decltype(ackQueue_)(); // we clear the ack queue now since all ids have become invalid
        }
      }
    }
  });
}

void Consumer::stop() noexcept {
  run_ = false;
}

void Consumer::join() noexcept {
  if (worker_.joinable())
    worker_.join();
}

Consumer::~Consumer() noexcept {
  stop();
  join();
}

bool Consumer::receive(Message& dst) {
  std::lock_guard<decltype(messageMtx_)> lk(messageMtx_);
  if (messages_.empty())
    return false;
  dst = messages_.front();
  messages_.pop();
  return true;
}

void Consumer::acknowledge(uint64_t id) {
  std::lock_guard<decltype(messageMtx_)> lk(ackMtx_);
  ackQueue_.emplace(id);
}

// Source end

// Main
using std::chrono::milliseconds;
using std::chrono::system_clock;

int main() {
  Consumer consumer("172.17.0.2", 5672, "consumer-test", "guest", "guest", vector<string>({"queue0"}));


  consumer.start();
  static const std::chrono::minutes kMaxIdle(5);

  vector<thread> processingThreads;

  // each thread will consume the queue until 5m idle
  for (size_t i = 0; i < 3; ++i) {
    processingThreads.emplace_back([&consumer] () {
      auto start = system_clock::now();
      Consumer::Message msg;
      do {
        if (consumer.receive(msg)) {
          cout << "Received: " << msg.body << endl;
          std::this_thread::sleep_for(milliseconds(100)); // simulates some work needed to be done by the thread
          consumer.acknowledge(msg.id);
          start = system_clock::now();
        }
      } while (system_clock::now() - start < kMaxIdle);
    });
  }

  for (auto& x : processingThreads) {
    x.join();
  }
}

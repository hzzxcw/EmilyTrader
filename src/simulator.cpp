#include "data_types.h"
#include "shm_ring_buffer.h"
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>

using namespace quote;

std::atomic<bool> running{true};

void order_producer(RingBuffer<MarketDataMsg, 1024 * 1024>* rb) {
    uint64_t seq = 1;
    while (running) {
        MarketDataMsg msg;
        msg.type = MsgType::Entrust;
        msg.data.order.channel_no = 1;
        msg.data.order.seq_no = seq++; // 注意：多生产者下，这里的 seq 仅作为模拟数据
        strncpy(msg.data.order.symbol, "600000.SH", 10);
        msg.data.order.price = 1050 + (rand() % 10);
        msg.data.order.volume = 100 * (1 + rand() % 10);
        msg.data.order.side = (rand() % 2 == 0) ? '1' : '2';
        msg.data.order.ord_type = '2';
        msg.data.order.timestamp = 20251228000000000 + seq;

        rb->push(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void trade_producer(RingBuffer<MarketDataMsg, 1024 * 1024>* rb) {
    uint64_t seq = 1000000;
    while (running) {
        MarketDataMsg msg;
        msg.type = MsgType::Execution;
        msg.data.exec.channel_no = 1;
        msg.data.exec.seq_no = seq++;
        strncpy(msg.data.exec.symbol, "600000.SH", 10);
        msg.data.exec.price = 1050 + (rand() % 10);
        msg.data.exec.volume = 100;
        msg.data.exec.bid_no = seq - 2;
        msg.data.exec.ask_no = seq - 1;
        msg.data.exec.exec_type = 'F';
        msg.data.exec.timestamp = 20251228000000000 + seq;

        rb->push(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

int main() {
    const std::string shm_name = "/quote_shm";
    const uint32_t capacity = 1024 * 1024;

    auto rb = ShmManager::create<MarketDataMsg, capacity>(shm_name);
    if (!rb) {
        return -1;
    }

    std::cout << "Simulator started (MPMC). Dual threads writing to: " << shm_name << std::endl;

    std::thread t1(order_producer, rb);
    std::thread t2(trade_producer, rb);

    // 运行一段时间或等待信号
    std::this_thread::sleep_for(std::chrono::seconds(600)); 
    running = false;
    
    t1.join();
    t2.join();

    return 0;
}

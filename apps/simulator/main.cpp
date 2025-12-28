#include "core/config.h"
#include "core/logger.h"
#include "market/market_writer.h"
#include <thread>
#include <iostream>

using namespace quote;

int main() {
    auto cfg = core::Config::load("config/config.json");
    core::Logger::init("simulator.log", cfg.log_level);
    
    Q_LOG_INFO("Simulator starting with SHM: {}", cfg.market_shm_name);
    
    auto journal = std::make_shared<core::Journal>(cfg.market_shm_name, true);
    market::MarketWriter writer(journal, 1001);

    uint64_t seq = 0;
    while (true) {
        core::TickOrder order;
        order.seq_no = ++seq;
        strncpy(order.symbol, "600000.SH", 10);
        order.price = 1000 + (rand() % 100);
        order.volume = 100;
        order.side = '1';

        writer.write_order(order, core::now_nano());
        
        if (seq % 1000 == 0) {
            Q_LOG_INFO("Simulated {} messages", seq);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

#include "core/config.h"
#include "core/logger.h"
#include "market/market_writer.h"
#include <thread>
#include <iostream>

using namespace quote;

int main(int argc, char* argv[]) {
    std::string config_path = "config/simulator.json";
    if (argc > 1) {
        config_path = argv[1];
    }
    auto cfg = core::Config::load(config_path);

    std::string log_file = "simulator.log";
    if (!cfg.log_file.empty() && cfg.log_file != "system.log") {
        log_file = cfg.log_file;
    }
    if (argc > 2) {
        log_file = argv[2];
    }

    core::Logger::init(log_file, cfg.log_level);
    
    Q_LOG_INFO("Simulator starting with SHM: {}", cfg.market_shm_name);
    
    auto journal = std::make_shared<core::Journal>(cfg.market_shm_name, true);
    market::MarketWriter writer(journal, 1001);

    uint64_t seq = 0;
    Q_LOG_INFO("Starting simulation loop...");
    while (true) {
        if (rand() % 2 == 0) {
            core::TickOrder order;
            order.seq_no = ++seq;
            strncpy(order.symbol, "600000.SH", 10);
            order.price = 1000 + (rand() % 100);
            order.volume = 100;
            order.side = (rand() % 2 == 0) ? '1' : '2';

            writer.write_order(order, core::now_nano());
        } else {
            core::TickExecution exec;
            exec.seq_no = ++seq;
            strncpy(exec.symbol, "600000.SH", 10);
            exec.price = 1000 + (rand() % 100);
            exec.volume = 100;
            exec.bid_no = seq; 
            exec.ask_no = seq; 

            writer.write_execution(exec, core::now_nano());
        }
        
        if (seq % 100 == 0) {
            Q_LOG_INFO("Simulated {} messages", seq);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

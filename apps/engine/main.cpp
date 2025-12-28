#include "core/config.h"
#include "core/logger.h"
#include "core/journal.h"
#include <iostream>
#include <thread>
#include <vector>

using namespace etrader;

int main(int argc, char* argv[]) {
    std::string config_path = "config/engine.json";
    if (argc > 1) config_path = argv[1];
    auto cfg = core::Config::load(config_path);

    std::string log_file = "engine.log";
    if (!cfg.log_file.empty() && cfg.log_file != "system.log") log_file = cfg.log_file;
    if (argc > 2) log_file = argv[2];

    core::Logger::init(log_file, cfg.log_level);
    Q_LOG_INFO("System Master (Journal Manager) starting...");

    // Create/Ensure existence of all journals
    // We open them as writer to ensure they are created and initialized.
    // In a real system, the master manages the memory layout.
    // Here we just ensure the SHM files exist.
    std::vector<std::shared_ptr<core::Journal>> journals;
    
    Q_LOG_INFO("Initializing Market Journal: {}", cfg.market_shm_name);
    journals.push_back(std::make_shared<core::Journal>(cfg.market_shm_name, true));
    
    Q_LOG_INFO("Initializing Strategy Journal: {}", cfg.strategy_shm_name);
    journals.push_back(std::make_shared<core::Journal>(cfg.strategy_shm_name, true));
    
    Q_LOG_INFO("Initializing Trade Journal: {}", cfg.trade_shm_name);
    journals.push_back(std::make_shared<core::Journal>(cfg.trade_shm_name, true));

    Q_LOG_INFO("System Master ready. Holding journals alive...");

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

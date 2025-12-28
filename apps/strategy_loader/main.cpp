#include "core/config.h"
#include "core/logger.h"
#include "core/poller.h"
#include "core/journal.h"
#include "strategy/base_strategy.h"
#include <iostream>
#include <memory>
#include <dlfcn.h>

using namespace quote;

typedef strategy::BaseStrategy* (*CreateStrategyFunc)();

int main(int argc, char* argv[]) {
    // Usage: strategy_loader [config_path]
    
    std::string config_path = "config/strategy.json";
    if (argc > 1) config_path = argv[1];
    auto cfg = core::Config::load(config_path);
    
    std::string log_file = "strategy.log";
    if (!cfg.log_file.empty() && cfg.log_file != "system.log") log_file = cfg.log_file;
    
    core::Logger::init(log_file, cfg.log_level);
    
    std::string strategy_so_path = cfg.strategy_so_path;
    if (strategy_so_path.empty()) {
        Q_LOG_ERROR("Strategy SO path not defined in config");
        return 1;
    }

    Q_LOG_INFO("Strategy Loader starting. Loading: {}", strategy_so_path);
    
    // Load .so
    void* handle = dlopen(strategy_so_path.c_str(), RTLD_LAZY);
    if (!handle) {
        Q_LOG_ERROR("Cannot open library: {}", dlerror());
        return 1;
    }
    
    // Load creator
    CreateStrategyFunc create_strategy = (CreateStrategyFunc)dlsym(handle, "create_strategy");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        Q_LOG_ERROR("Cannot load symbol 'create_strategy': {}", dlsym_error);
        dlclose(handle);
        return 1;
    }
    
    // Create Strategy
    strategy::BaseStrategy* strategy_ptr = create_strategy();
    std::shared_ptr<strategy::BaseStrategy> strategy(strategy_ptr); 
    
    // Create Context
    // Reads Market & Trade Journals
    auto mkt_journal = std::make_shared<core::Journal>(cfg.market_shm_name, false);
    auto trade_journal = std::make_shared<core::Journal>(cfg.trade_shm_name, false);
    
    // Writes Strategy Journal
    auto str_journal = std::make_shared<core::Journal>(cfg.strategy_shm_name, true);
    
    auto poller = std::make_shared<core::Poller>();
    poller->add_journal(mkt_journal);
    poller->add_journal(trade_journal);
    
    strategy::StrategyContext ctx(str_journal);
    
    strategy->on_init(ctx, cfg.json_content);
    Q_LOG_INFO("Strategy initialized. Running loop...");
    
    while (true) {
        poller->poll([&](const core::Frame& frame) {
            core::Frame local_frame = frame;
            local_frame.header.recv_time = core::now_nano();
            
            if (frame.header.msg_type == core::MsgType::Entrust || frame.header.msg_type == core::MsgType::Execution) {
                strategy->on_tick(ctx, local_frame);
            } else if (frame.header.msg_type == core::MsgType::OrderResponse || frame.header.msg_type == core::MsgType::ExecReport) {
                strategy->on_response(ctx, local_frame);
            }
        });
    }
    
    dlclose(handle);
    return 0;
}

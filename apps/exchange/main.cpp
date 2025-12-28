#include "core/config.h"
#include "core/logger.h"
#include "core/poller.h"
#include "core/journal.h"
#include "trade/mock_trade.h"
#include <iostream>
#include <memory>

using namespace quote;

int main(int argc, char* argv[]) {
    std::string config_path = "config/exchange.json";
    if (argc > 1) config_path = argv[1];
    auto cfg = core::Config::load(config_path);

    std::string log_file = "exchange.log";
    if (!cfg.log_file.empty() && cfg.log_file != "system.log") log_file = cfg.log_file;
    if (argc > 2) log_file = argv[2];

    core::Logger::init(log_file, cfg.log_level);
    Q_LOG_INFO("Exchange Service starting...");

    // Writer to Trade Journal
    auto trade_journal = std::make_shared<core::Journal>(cfg.trade_shm_name, true);
    auto trade_module = std::make_shared<trade::MockTrade>(trade_journal);

    // Reader from Strategy Journal
    auto str_journal = std::make_shared<core::Journal>(cfg.strategy_shm_name, false);

    // Poller
    auto poller = std::make_shared<core::Poller>();
    poller->add_journal(str_journal);

    Q_LOG_INFO("Exchange Service running. Listening on {}", cfg.strategy_shm_name);

    while (true) {
        poller->poll([&](const core::Frame& frame) {
            if (frame.header.msg_type == core::MsgType::OrderInput) {
                const auto& input = frame.as<core::OrderInput>();
                trade_module->place_order(input);
            }
        });
    }

    return 0;
}

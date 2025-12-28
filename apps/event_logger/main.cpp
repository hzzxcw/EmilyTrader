#include "core/config.h"
#include "core/logger.h"
#include "core/poller.h"
#include "event/event_logger.h"
#include <iostream>

using namespace quote;

int main() {
    auto cfg = core::Config::load("config/config.json");
    core::Logger::init("event_logger.log", cfg.log_level);

    auto mkt_journal = std::make_shared<core::Journal>(cfg.market_shm_name, false);
    auto trade_journal = std::make_shared<core::Journal>(cfg.trade_shm_name, false);
    auto str_journal = std::make_shared<core::Journal>(cfg.strategy_shm_name, false);

    auto poller = std::make_shared<core::Poller>();
    poller->add_journal(mkt_journal);
    poller->add_journal(trade_journal);
    poller->add_journal(str_journal);

    event::EventLogger logger(poller);
    
    Q_LOG_INFO("Event Logger started. Monitoring all journals...");
    logger.run();

    return 0;
}

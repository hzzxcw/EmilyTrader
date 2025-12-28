#include "core/config.h"
#include "core/logger.h"
#include "core/poller.h"
#include "strategy/base_strategy.h"
#include <iostream>

using namespace quote;

class MyStrategy : public strategy::BaseStrategy {
public:
    void on_init(strategy::StrategyContext& ctx) override {
        Q_LOG_INFO("MyStrategy initialized");
    }

    void on_tick(strategy::StrategyContext& ctx, const core::Frame& frame) override {
        const auto& order = frame.as<core::TickOrder>();
        // 简单策略：价格低于 1010 就下单
        if (order.price < 1010) {
            core::OrderInput input;
            input.order_id = order.seq_no;
            strncpy(input.symbol, order.symbol, 10);
            input.price = order.price;
            input.volume = 100;
            input.side = '1';
            
            // 填充审计所需的时间戳，现在直接从 frame.header 中获取
            input.trigger_t1 = frame.header.push_time;
            input.trigger_t2 = frame.header.recv_time;
            
            ctx.send_order(input);
        }
    }

    void on_response(strategy::StrategyContext& ctx, const core::Frame& frame) override {
        if (frame.header.msg_type == core::MsgType::OrderResponse) {
            const auto& resp = frame.as<core::OrderResponse>();
            // Q_LOG_INFO("Received OrderResponse for ID: {}, Success: {}", resp.order_id, resp.success);
        }
    }
};

int main() {
    auto cfg = core::Config::load("config/config.json");
    core::Logger::init("engine.log", cfg.log_level);

    auto mkt_journal = std::make_shared<core::Journal>(cfg.market_shm_name, false);
    auto trade_journal = std::make_shared<core::Journal>(cfg.trade_shm_name, false);
    auto str_journal = std::make_shared<core::Journal>(cfg.strategy_shm_name, true);

    auto poller = std::make_shared<core::Poller>();
    poller->add_journal(mkt_journal);
    poller->add_journal(trade_journal);

    // Mock Trade Module (In-process for this demo, but writes to SHM)
    auto trade_module = std::make_shared<trade::MockTrade>(trade_journal);

    strategy::StrategyContext ctx(poller, str_journal, trade_module);
    MyStrategy strategy;
    strategy.on_init(ctx);

    Q_LOG_INFO("Trading Engine running...");

    while (true) {
        ctx.poll([&](const core::Frame& frame) {
            // 制作本地拷贝以携带接收时间戳 (T2)
            core::Frame local_frame = frame;
            local_frame.header.recv_time = core::now_nano();
            
            if (frame.header.msg_type == core::MsgType::Entrust || frame.header.msg_type == core::MsgType::Execution) {
                strategy.on_tick(ctx, local_frame);
            } else {
                strategy.on_response(ctx, local_frame);
            }
        });
    }

    return 0;
}

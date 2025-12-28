#pragma once

#include "core/common.h"
#include "core/journal.h"
#include "core/poller.h"
#include "trade/mock_trade.h"
#include <memory>

namespace quote {
namespace strategy {

class StrategyContext {
public:
    StrategyContext(std::shared_ptr<core::Poller> poller, 
                   std::shared_ptr<core::Journal> strategy_journal,
                   std::shared_ptr<trade::MockTrade> trade_module)
        : poller_(poller), writer_(strategy_journal), trade_(trade_module) {}

    void send_order(const core::OrderInput& input, core::nano_t trigger_t1, core::nano_t trigger_t2) {
        // 计算下单延迟 (T3)
        core::nano_t t3 = core::now_nano();
        core::LatencyStats stats;
        stats.type = core::MsgType::OrderInput;
        stats.seq_or_id = input.order_id;
        stats.t1 = trigger_t1;
        stats.t2 = trigger_t2;
        stats.t3 = t3;
        
        // 写入策略通道（用于 Telemetry）
        writer_->write(core::MsgType::LatencyStats, 1, t3, &stats, sizeof(stats));
        
        // 真正下单
        trade_->place_order(input);
    }

    void poll(std::function<void(const core::Frame&)> callback) {
        poller_->poll(callback);
    }

private:
    std::shared_ptr<core::Poller> poller_;
    std::shared_ptr<core::Journal> writer_;
    std::shared_ptr<trade::MockTrade> trade_;
};

class BaseStrategy {
public:
    virtual ~BaseStrategy() = default;
    virtual void on_init(StrategyContext& ctx) = 0;
    virtual void on_tick(StrategyContext& ctx, const core::Frame& frame, core::nano_t t2) = 0;
    virtual void on_response(StrategyContext& ctx, const core::Frame& frame) = 0;
};

} // namespace strategy
} // namespace quote

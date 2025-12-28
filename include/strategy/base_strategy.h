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

    // 接口已简化：不再显式传递时间戳，直接从 input 中读取
    void send_order(const core::OrderInput& input) {
        core::nano_t t3 = core::now_nano();
        
        core::LatencyStats stats;
        stats.type = core::MsgType::OrderInput;
        stats.seq_or_id = input.order_id;
        stats.t1 = input.trigger_t1;
        stats.t2 = input.trigger_t2;
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
    // T2 已包含在 FrameHeader 中，无需单独传递
    virtual void on_tick(StrategyContext& ctx, const core::Frame& frame) = 0;
    virtual void on_response(StrategyContext& ctx, const core::Frame& frame) = 0;
};

} // namespace strategy
} // namespace quote

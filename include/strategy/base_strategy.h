#pragma once

#include "core/common.h"
#include "core/journal.h"
#include "core/poller.h"
#include "core/data_types.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace etrader {
namespace strategy {

class StrategyContext {
public:
    StrategyContext(std::shared_ptr<core::Journal> strategy_journal)
        : writer_(strategy_journal) {}

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
        
        // 写入策略通道（下单请求）
        writer_->write(core::MsgType::OrderInput, input.order_id, t3, &input, sizeof(input));
    }

private:
    std::shared_ptr<core::Journal> writer_;
};

class BaseStrategy {
public:
    virtual ~BaseStrategy() = default;
    virtual void on_init(StrategyContext& ctx, const nlohmann::json& config) = 0;
    // T2 已包含在 FrameHeader 中，无需单独传递
    virtual void on_tick(StrategyContext& ctx, const core::Frame& frame) = 0;
    virtual void on_response(StrategyContext& ctx, const core::Frame& frame) = 0;
};

} // namespace strategy
} // namespace etrader

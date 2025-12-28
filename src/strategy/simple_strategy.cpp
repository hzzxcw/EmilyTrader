#include "strategy/base_strategy.h"
#include "orderbook/orderbook.h"
#include <cmath>
#include <iostream>
#include <cstring>
#include "core/logger.h"

namespace quote {
namespace strategy {

class SimpleStrategy : public BaseStrategy {
public:
    void on_init(StrategyContext& ctx, const nlohmann::json& config) override {
        Q_LOG_INFO("SimpleStrategy initialized with heavy workload simulation");
        if (config.contains("max_order_price")) {
            int max_price = config["max_order_price"];
            Q_LOG_INFO("Config: max_order_price = {}", max_price);
        }
    }

    void on_tick(StrategyContext& ctx, const core::Frame& frame) override {
        // 1. 真实维护订单簿
        if (frame.header.msg_type == core::MsgType::Entrust) {
            ob_.on_order(frame.as<core::TickOrder>());
        } else if (frame.header.msg_type == core::MsgType::Execution) {
            ob_.on_execution(frame.as<core::TickExecution>());
        }
        
        if (rand() % 50 == 0) {
             Q_LOG_INFO("OrderBook Snapshot:\n{}", ob_.dump_top(3));
        }

        // 2. 模拟复杂的因子计算负载 (Synthetic Workload)
        // 执行一段繁忙循环，模拟约 1-5 微秒的策略耗时
        double dummy = 0.0;
        for (int i = 0; i < 5000; ++i) {
            dummy += std::sin(i) * std::cos(i);
        }
        (void)dummy; // 防止编译器优化

        const auto& order = frame.as<core::TickOrder>();
        // 简单策略：根据订单簿价格决策
        if (order.price < 1010) {
            core::OrderInput input;
            input.order_id = order.seq_no;
            strncpy(input.symbol, order.symbol, 10);
            input.price = order.price;
            input.volume = 100;
            input.side = '1';
            
            input.trigger_t1 = frame.header.push_time;
            input.trigger_t2 = frame.header.recv_time;
            
            ctx.send_order(input);
        }
    }

    void on_response(StrategyContext& ctx, const core::Frame& frame) override {
        // ... 回报处理
    }

private:
    OrderBook ob_;
};

} // namespace strategy
} // namespace quote

extern "C" {
    quote::strategy::BaseStrategy* create_strategy() {
        return new quote::strategy::SimpleStrategy();
    }
}

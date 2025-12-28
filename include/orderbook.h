#pragma once

#include "core/data_types.h"
#include <map>
#include <vector>
#include <iostream>

namespace quote {

class OrderBook {
public:
    void on_order(const core::TickOrder& order);
    void on_execution(const core::TickExecution& exec);
    void display_top(int levels = 5) const;

    // 获取盘口中间价
    double get_mid_price() const;

private:
    // 价格 -> 数量
    std::map<uint32_t, uint64_t, std::greater<uint32_t>> bids_;
    std::map<uint32_t, uint64_t, std::less<uint32_t>> asks_;

    // 存储未成交的订单 (简化：SeqNo -> Order)
    std::map<uint64_t, core::TickOrder> orders_;
};

} // namespace quote

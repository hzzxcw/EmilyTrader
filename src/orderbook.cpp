#include "orderbook.h"
#include <iomanip>

namespace quote {

void OrderBook::on_order(const TickOrder& order) {
    orders_[order.seq_no] = order;
    if (order.side == '1') {
        bids_[order.price] += order.volume;
    } else {
        asks_[order.price] += order.volume;
    }
}

void OrderBook::on_execution(const TickExecution& exec) {
    if (exec.exec_type == 'F') { // 成交
        // 简化处理：从买卖双方减少量
        // 在真实场景中，成交会对应具体的委托序号
        // 这里为了演示，简单更新对应价格的挂单量
        
        // 尝试从 orders 找到原始委托以确定价格（如果是逐笔成交数据通常包含价格）
        if (bids_.count(exec.price)) {
            if (bids_[exec.price] >= exec.volume) bids_[exec.price] -= exec.volume;
            else bids_[exec.price] = 0;
            if (bids_[exec.price] == 0) bids_.erase(exec.price);
        }
        if (asks_.count(exec.price)) {
            if (asks_[exec.price] >= exec.volume) asks_[exec.price] -= exec.volume;
            else asks_[exec.price] = 0;
            if (asks_[exec.price] == 0) asks_.erase(exec.price);
        }
    } else if (exec.exec_type == '4') { // 撤单
        // 撤单通常针对某一个委托序号
        auto it = orders_.find(exec.bid_no > 0 ? exec.bid_no : exec.ask_no);
        if (it != orders_.end()) {
            uint32_t price = it->second.price;
            if (it->second.side == '1') {
                if (bids_[price] >= exec.volume) bids_[price] -= exec.volume;
                if (bids_[price] == 0) bids_.erase(price);
            } else {
                if (asks_[price] >= exec.volume) asks_[price] -= exec.volume;
                if (asks_[price] == 0) asks_.erase(price);
            }
            orders_.erase(it);
        }
    }
}

double OrderBook::get_mid_price() const {
    if (bids_.empty() || asks_.empty()) return 0.0;
    return (bids_.begin()->first + asks_.begin()->first) / 2.0;
}

void OrderBook::display_top(int levels) const {
    std::cout << "--- OrderBook Top " << levels << " ---" << std::endl;
    int count = 0;
    auto it_ask = asks_.rbegin(); // 卖单价格从高到低显示一部分，或者直接显示卖1
    // 通常卖1在最下面
    std::vector<std::pair<uint32_t, uint64_t>> tmp_asks;
    for (auto it = asks_.begin(); it != asks_.end() && count < levels; ++it, ++count) {
        tmp_asks.push_back(*it);
    }
    for (int i = tmp_asks.size() - 1; i >= 0; --i) {
        std::cout << "ASK " << i+1 << ": " << std::setw(8) << tmp_asks[i].first << " | " << tmp_asks[i].second << std::endl;
    }

    std::cout << "-----------------------" << std::endl;
    count = 0;
    for (auto it = bids_.begin(); it != bids_.end() && count < levels; ++it, ++count) {
        std::cout << "BID " << count+1 << ": " << std::setw(8) << it->first << " | " << it->second << std::endl;
    }
    std::cout << "-----------------------" << std::endl;
}

} // namespace quote

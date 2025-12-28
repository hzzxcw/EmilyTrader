#include "orderbook.h"
#include <iomanip>

namespace quote {

void OrderBook::on_order(const core::TickOrder& order) {
    orders_[order.seq_no] = order;
    if (order.side == '1') {
        bids_[order.price] += order.volume;
    } else {
        asks_[order.price] += order.volume;
    }
}

void OrderBook::on_execution(const core::TickExecution& exec) {
    // 简化处理：尝试从买卖双方减少量
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
}

double OrderBook::get_mid_price() const {
    if (bids_.empty() || asks_.empty()) return 0.0;
    return (bids_.begin()->first + asks_.begin()->first) / 2.0;
}

void OrderBook::display_top(int levels) const {
    std::cout << "--- OrderBook Top " << levels << " ---" << std::endl;
    int count = 0;
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

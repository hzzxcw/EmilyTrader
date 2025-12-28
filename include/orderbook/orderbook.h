#pragma once

#include "core/data_types.h"
#include <map>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

namespace quote {

class OrderBook {
public:
    inline void on_order(const core::TickOrder& order) {
        orders_[order.seq_no] = order;
        if (order.side == '1') {
            bids_[order.price] += order.volume;
        } else {
            asks_[order.price] += order.volume;
        }
    }

    inline void on_execution(const core::TickExecution& exec) {
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

    inline double get_mid_price() const {
        if (bids_.empty() || asks_.empty()) return 0.0;
        return (bids_.begin()->first + asks_.begin()->first) / 2.0;
    }

    inline std::string dump_top(int levels = 5) const {
        std::stringstream ss;
        ss << "--- OrderBook Top " << levels << " ---" << "\n";
        int count = 0;
        std::vector<std::pair<uint32_t, uint64_t>> tmp_asks;
        for (auto it = asks_.begin(); it != asks_.end() && count < levels; ++it, ++count) {
            tmp_asks.push_back(*it);
        }
        for (int i = tmp_asks.size() - 1; i >= 0; --i) {
            ss << "ASK " << i+1 << ": " << std::setw(8) << tmp_asks[i].first << " | " << tmp_asks[i].second << "\n";
        }

        ss << "-----------------------" << "\n";
        count = 0;
        for (auto it = bids_.begin(); it != bids_.end() && count < levels; ++it, ++count) {
            ss << "BID " << count+1 << ": " << std::setw(8) << it->first << " | " << it->second << "\n";
        }
        ss << "-----------------------";
        return ss.str();
    }

private:
    std::map<uint32_t, uint64_t, std::greater<uint32_t>> bids_;
    std::map<uint32_t, uint64_t, std::less<uint32_t>> asks_;
    std::map<uint64_t, core::TickOrder> orders_;
};

} // namespace quote

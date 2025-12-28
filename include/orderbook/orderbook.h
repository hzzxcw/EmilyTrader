#pragma once

#include "core/data_types.h"
#include "core/logger.h"
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace quote {

class OrderBook {
public:
    inline void on_order(const core::TickOrder& order) {
        // Store the order for reference
        orders_[order.seq_no] = order;

        // Add to appropriate price queue (maintaining time priority)
        if (order.side == '1') { // Bid (买单)
            bids_[order.price].push_back(order);
            Q_LOG_DEBUG("OrderBook: Add BID order seq={} price={} volume={}", 
                order.seq_no, order.price, order.volume);
        } else { // Ask (卖单)
            asks_[order.price].push_back(order);
            Q_LOG_DEBUG("OrderBook: Add ASK order seq={} price={} volume={}", 
                order.seq_no, order.price, order.volume);
        }
    }

    inline void on_execution(const core::TickExecution& exec) {
        // Chinese A-share executions specify which orders were matched via bid_no and ask_no
        // Remove the matched orders from the orderbook
        
        Q_LOG_DEBUG("OrderBook: Execution price={} volume={} bid_no={} ask_no={}", 
            exec.price, exec.volume, exec.bid_no, exec.ask_no);

        // Handle bid order execution (买单成交)
        if (exec.bid_no != 0) {
            Q_LOG_DEBUG("OrderBook: Remove BID order seq={} due to execution", exec.bid_no);
            remove_order_from_queues(exec.bid_no);
        }

        // Handle ask order execution (卖单成交)
        if (exec.ask_no != 0) {
            Q_LOG_DEBUG("OrderBook: Remove ASK order seq={} due to execution", exec.ask_no);
            remove_order_from_queues(exec.ask_no);
        }

        // Clean up empty price levels
        clean_empty_price_levels();
    }

    inline double get_mid_price() const {
        if (bids_.empty() || asks_.empty()) return 0.0;
        return (bids_.begin()->first + asks_.begin()->first) / 2.0;
    }

    inline std::string dump_top(int levels = 5) const {
        std::stringstream ss;
        ss << "--- OrderBook Top " << levels << " ---" << "\n";

        // Display asks (卖盘) from best to worst (low to high price)
        int count = 0;
        for (auto it = asks_.begin(); it != asks_.end() && count < levels; ++it, ++count) {
            uint64_t total_volume = 0;
            for (const auto& order : it->second) {
                total_volume += order.volume;
            }
            ss << "ASK " << count+1 << ": " << std::setw(8) << it->first << " | " << total_volume << "\n";
        }

        ss << "-----------------------" << "\n";

        // Display bids (买盘) from best to worst (high to low price)
        count = 0;
        for (auto it = bids_.begin(); it != bids_.end() && count < levels; ++it, ++count) {
            uint64_t total_volume = 0;
            for (const auto& order : it->second) {
                total_volume += order.volume;
            }
            ss << "BID " << count+1 << ": " << std::setw(8) << it->first << " | " << total_volume << "\n";
        }
        ss << "-----------------------";
        return ss.str();
    }

private:
    // Remove a specific order from the price queues
    inline void remove_order_from_queues(uint64_t seq_no) {
        auto order_it = orders_.find(seq_no);
        if (order_it == orders_.end()) return;

        const auto& order = order_it->second;

        // Find and remove from appropriate price queue
        if (order.side == '1') { // Bid order
            auto price_it = bids_.find(order.price);
            if (price_it != bids_.end()) {
                auto& queue = price_it->second;
                // Remove the specific order (orders are unique by seq_no)
                auto remove_it = std::find_if(queue.begin(), queue.end(),
                    [seq_no](const core::TickOrder& o) { return o.seq_no == seq_no; });
                if (remove_it != queue.end()) {
                    queue.erase(remove_it);
                }
            }
        } else { // Ask order
            auto price_it = asks_.find(order.price);
            if (price_it != asks_.end()) {
                auto& queue = price_it->second;
                // Remove the specific order (orders are unique by seq_no)
                auto remove_it = std::find_if(queue.begin(), queue.end(),
                    [seq_no](const core::TickOrder& o) { return o.seq_no == seq_no; });
                if (remove_it != queue.end()) {
                    queue.erase(remove_it);
                }
            }
        }

        // Remove from orders map
        orders_.erase(order_it);
    }

    // Clean up empty price levels
    inline void clean_empty_price_levels() {
        // Clean bids
        for (auto it = bids_.begin(); it != bids_.end(); ) {
            if (it->second.empty()) {
                it = bids_.erase(it);
            } else {
                ++it;
            }
        }

        // Clean asks
        for (auto it = asks_.begin(); it != asks_.end(); ) {
            if (it->second.empty()) {
                it = asks_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    // Price -> queue of orders (maintaining time priority)
    // Bids: higher price first (std::greater for price comparison)
    std::map<uint32_t, std::deque<core::TickOrder>, std::greater<uint32_t>> bids_;
    // Asks: lower price first (std::less for price comparison)
    std::map<uint32_t, std::deque<core::TickOrder>, std::less<uint32_t>> asks_;
    // Order lookup by sequence number
    std::map<uint64_t, core::TickOrder> orders_;
};

} // namespace quote

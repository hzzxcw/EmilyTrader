#pragma once

#include "core/journal.h"
#include "core/data_types.h"
#include <map>
#include <string>

namespace etrader {
namespace trade {

class MockTrade {
public:
    MockTrade(std::shared_ptr<core::Journal> response_journal)
        : journal_(response_journal) {
        cash_ = 1000000.0; // 初始 100w 资金
    }

    void place_order(const core::OrderInput& input) {
        // 模拟下单逻辑
        core::OrderResponse resp;
        resp.order_id = input.order_id;
        
        if (cash_ < input.price * input.volume) {
            resp.success = false;
            strncpy(resp.error_msg, "Insufficient funds", 32);
        } else {
            resp.success = true;
            cash_ -= input.price * input.volume;
            positions_[input.symbol] += input.volume;
            strncpy(resp.error_msg, "Success", 32);
        }
        
        // 写入响应通道
        journal_->write(core::MsgType::OrderResponse, 999, core::now_nano(), &resp, sizeof(resp));
        
        // 如果成功，模拟立即成交
        if (resp.success) {
            core::ExecReport report;
            report.order_id = input.order_id;
            report.exec_price = input.price;
            report.exec_volume = input.volume;
            journal_->write(core::MsgType::ExecReport, 999, core::now_nano(), &report, sizeof(report));
        }
    }

    double get_cash() const { return cash_; }
    uint64_t get_position(const std::string& symbol) { return positions_[symbol]; }

private:
    std::shared_ptr<core::Journal> journal_;
    double cash_;
    std::map<std::string, uint64_t> positions_;
};

} // namespace trade
} // namespace etrader

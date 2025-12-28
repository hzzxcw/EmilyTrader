#pragma once

#include "core/journal.h"
#include "core/data_types.h"
#include <memory>

namespace quote {
namespace market {

class MarketWriter {
public:
    MarketWriter(std::shared_ptr<core::Journal> journal, uint32_t source_id)
        : journal_(journal), source_id_(source_id) {}

    void write_order(const core::TickOrder& order, core::nano_t t0) {
        journal_->write(core::MsgType::Entrust, source_id_, t0, &order, sizeof(order));
    }

    void write_execution(const core::TickExecution& exec, core::nano_t t0) {
        journal_->write(core::MsgType::Execution, source_id_, t0, &exec, sizeof(exec));
    }

private:
    std::shared_ptr<core::Journal> journal_;
    uint32_t source_id_;
};

} // namespace market
} // namespace quote

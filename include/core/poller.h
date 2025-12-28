#pragma once

#include "journal.h"
#include <vector>
#include <memory>
#include <functional>

namespace quote {
namespace core {

class Poller {
public:
    struct ReaderState {
        std::shared_ptr<Journal> journal;
        uint64_t next_idx;
    };

    void add_journal(std::shared_ptr<Journal> journal) {
        // 从当前位置开始读，或者从头读
        states_.push_back({journal, journal->get_write_idx()});
    }

    void poll(std::function<void(const Frame&)> callback) {
        bool found = false;
        for (auto& state : states_) {
            uint64_t write_idx = state.journal->get_write_idx();
            while (state.next_idx < write_idx) {
                callback(state.journal->read(state.next_idx));
                state.next_idx++;
                found = true;
            }
        }
        if (!found) {
            cpu_pause();
        }
    }

private:
    std::vector<ReaderState> states_;
};

} // namespace core
} // namespace quote

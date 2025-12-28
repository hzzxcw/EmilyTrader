#pragma once

#include "core/poller.h"
#include "core/data_types.h"
#include <fstream>
#include <iostream>

namespace quote {
namespace event {

class EventLogger {
public:
    EventLogger(std::shared_ptr<core::Poller> poller) : poller_(poller) {
        latency_file_.open("latency_stats.csv");
        latency_file_ << "Type,ID,T0,T1,T2,T3,Ingest_Lat,Order_Lat\n";
        
        trade_file_.open("trade_events.csv");
        trade_file_ << "Type,OrderID,Price,Volume,Status\n";

        market_file_.open("market_data.csv");
        market_file_ << "Type,SeqNo,Symbol,Price,Volume,Extra\n";
    }

    void run() {
        while (true) {
            poller_->poll([this](const core::Frame& frame) {
                process_frame(frame);
            });
        }
    }

private:
    void process_frame(const core::Frame& frame) {
        auto type = frame.header.msg_type;
        
        if (type == core::MsgType::LatencyStats) {
            const auto& stats = frame.as<core::LatencyStats>();
            latency_file_ << (int)stats.type << "," << stats.seq_or_id << ","
                         << stats.t0 << "," << stats.t1 << "," << stats.t2 << "," << stats.t3 << ","
                         << (stats.t1 - stats.t0) << "," << (stats.t3 - stats.t1) << "\n";
            latency_file_.flush();
        } else if (type == core::MsgType::OrderResponse) {
            const auto& resp = frame.as<core::OrderResponse>();
            trade_file_ << "Response," << resp.order_id << ",0,0," << (resp.success ? "OK" : resp.error_msg) << "\n";
            trade_file_.flush();
        } else if (type == core::MsgType::ExecReport) {
            const auto& report = frame.as<core::ExecReport>();
            trade_file_ << "Exec," << report.order_id << "," << report.exec_price << "," << report.exec_volume << ",Done\n";
            trade_file_.flush();
        } else if (type == core::MsgType::Entrust) {
            const auto& order = frame.as<core::TickOrder>();
            market_file_ << "Entrust," << order.seq_no << "," << order.symbol << "," 
                         << order.price << "," << order.volume << "," << order.side << "\n";
            market_file_.flush();
        } else if (type == core::MsgType::Execution) {
            const auto& exec = frame.as<core::TickExecution>();
            market_file_ << "Execution," << exec.seq_no << "," << exec.symbol << "," 
                         << exec.price << "," << exec.volume << "," 
                         << exec.bid_no << "|" << exec.ask_no << "\n";
            market_file_.flush();
        }
    }

    std::shared_ptr<core::Poller> poller_;
    std::ofstream latency_file_;
    std::ofstream trade_file_;
    std::ofstream market_file_;
};

} // namespace event
} // namespace quote

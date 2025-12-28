#pragma once

#include "common.h"
#include <cstdint>
#include <cstring>

namespace quote {
namespace core {

// --- Market Data ---
struct TickOrder {
    uint64_t seq_no;
    char symbol[10];
    uint32_t price;
    uint64_t volume;
    char side;
};

struct TickExecution {
    uint64_t seq_no;
    char symbol[10];
    uint32_t price;
    uint64_t volume;
    uint64_t bid_no;
    uint64_t ask_no;
};

// --- Trade Requests ---
struct OrderInput {
    uint64_t order_id;
    char symbol[10];
    uint32_t price;
    uint64_t volume;
    char side;

    // 延迟审计字段：记录该订单是由哪个时间点的行情触发的
    nano_t trigger_t1;
    nano_t trigger_t2;
};

// --- Trade Responses ---
struct OrderResponse {
    uint64_t order_id;
    bool success;
    char error_msg[32];
};

struct ExecReport {
    uint64_t order_id;
    uint32_t exec_price;
    uint64_t exec_volume;
};

// --- Telemetry ---
struct LatencyStats {
    MsgType type;
    uint64_t seq_or_id;
    nano_t t0; // Original source time
    nano_t t1; // Ingest time
    nano_t t2; // Strategy receive time
    nano_t t3; // Action time
};

} // namespace core
} // namespace quote

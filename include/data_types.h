#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace quote {

enum class MsgType : uint8_t {
    Entrust = 1,    // 逐笔委托
    Execution = 2   // 逐笔成交
};

// 逐笔委托 (Tick Order)
struct TickOrder {
    uint32_t channel_no;   // 频道号
    uint64_t seq_no;       // 序列号
    char symbol[10];       // 证券代码
    uint32_t price;        // 价格 (单位：分，实际价格 * 10000 或 100 视具体协议)
    uint64_t volume;       // 数量
    char side;             // 买卖方向 ('1'=买, '2'=卖, 'G'=借, 'F'=贷)
    char ord_type;         // 订单类型 ('1'=市价, '2'=限价, 'U'=本方最优)
    uint64_t timestamp;    // 时间戳 (HHMMSSsss)
};

// 逐笔成交 (Tick Execution)
struct TickExecution {
    uint32_t channel_no;   // 频道号
    uint64_t seq_no;       // 序列号
    char symbol[10];       // 证券代码
    uint32_t price;        // 成交价格
    uint64_t volume;       // 成交数量
    uint64_t bid_no;       // 买方委托序号
    uint64_t ask_no;       // 卖方委托序号
    char exec_type;        // 成交类型 ('4'=撤单, 'F'=成交)
    uint64_t timestamp;    // 时间戳
};

// 统一封装消息，对齐到 64 字节以避免伪共享并提高缓存命中率
struct alignas(64) MarketDataMsg {
    MsgType type;
    union {
        TickOrder order;
        TickExecution exec;
    } data;
};

} // namespace quote

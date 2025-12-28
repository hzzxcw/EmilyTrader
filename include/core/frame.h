#pragma once

#include "common.h"
#include <cstring>

namespace etrader {
namespace core {

struct FrameHeader {
    uint32_t source_id;
    MsgType msg_type;
    nano_t gen_time;      // 消息产生时间 (T0)
    nano_t push_time;     // 进入 SHM 时间 (T1)
    nano_t recv_time;     // 消费者接收时间 (T2)
    uint32_t data_len;
};

// 预留足够大的 Payload 空间，以容纳不同的数据结构
constexpr size_t MAX_FRAME_PAYLOAD_SIZE = 256;

struct alignas(64) Frame {
    FrameHeader header;
    uint8_t payload[MAX_FRAME_PAYLOAD_SIZE];

    template<typename T>
    const T& as() const {
        return *reinterpret_cast<const T*>(payload);
    }

    template<typename T>
    void set_data(const T& data) {
        header.data_len = sizeof(T);
        std::memcpy(payload, &data, sizeof(T));
    }
};

} // namespace core
} // namespace etrader

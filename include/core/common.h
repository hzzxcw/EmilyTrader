#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace quote {
namespace core {

// 使用 PAUSE 指令优化忙等
inline void cpu_pause() {
#if defined(__x86_64__) || defined(_M_X64)
    _mm_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    __asm__ __volatile__("yield" ::: "memory");
#endif
}

using nano_t = int64_t;

inline nano_t now_nano() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

enum class MsgType : uint16_t {
    // Market Data
    Entrust = 1,
    Execution = 2,
    
    // Trade Requests
    OrderInput = 101,
    OrderAction = 102,
    
    // Trade Responses
    OrderResponse = 201,
    ExecReport = 202,
    
    // Telemetry
    LatencyStats = 301,
    
    // System
    SystemExit = 999
};

} // namespace core
} // namespace quote

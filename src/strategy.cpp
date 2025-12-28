#include "data_types.h"
#include "shm_ring_buffer.h"
#include "orderbook.h"
#include <iostream>
#include <thread>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/FileSink.h>
#include <quill/sinks/ConsoleSink.h>

using namespace quote;

int main() {
    const std::string shm_name = "/quote_shm";
    const uint32_t capacity = 1024 * 1024;

    auto rb = ShmManager::attach<MarketDataMsg, capacity>(shm_name);
    while (!rb) {
        std::cout << "Waiting for simulator to create shared memory..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        rb = ShmManager::attach<MarketDataMsg, capacity>(shm_name);
    }

    // 初始化 Quill
    quill::Backend::start();

    // 创建文件和控制台输出的 Sinks
    // 第一个参数会被用作文件名
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>("strategy_trade.log");
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");
    
    // 设置自定义格式
    quill::PatternFormatterOptions fmt_opts;
    fmt_opts.format_pattern = "[%(time)] [%(log_level)] %(message)";
    fmt_opts.timestamp_pattern = "%H:%M:%S.%Qus";

    quill::Logger* logger = quill::Frontend::create_or_get_logger("strategy_logger", {file_sink, console_sink}, fmt_opts);
    logger->set_log_level(quill::LogLevel::TraceL3);
    
    LOG_INFO(logger, "Strategy Processor started (Quill MPMC).");

    OrderBook ob;
    uint64_t read_idx = 0;

    double last_mid = 0;

    while (true) {
        // 使用无锁等待获取数据
        const auto& msg = rb->wait_and_get(read_idx);
        
        if (msg.type == MsgType::Entrust) {
            ob.on_order(msg.data.order);
        } else {
            ob.on_execution(msg.data.exec);
        }

        read_idx++;

        // 策略逻辑：每处理 100 条消息检查一次盘口
        if (read_idx % 100 == 0) {
            double mid = ob.get_mid_price();
            if (mid > 0) {
                if (last_mid > 0 && std::abs(mid - last_mid) > 0.5) {
                    LOG_WARNING(logger, "Price change detected! Mid: {} Previous: {}", mid, last_mid);
                }
                last_mid = mid;
            }
        }
    }

    return 0;
}

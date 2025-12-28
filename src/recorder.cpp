#include "data_types.h"
#include "shm_ring_buffer.h"
#include <iostream>
#include <thread>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/FileSink.h>

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

    // 创建一个直接写入文件的 Sink
    quill::FileSinkConfig cfg;
    cfg.set_open_mode('w');
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>("market_data.csv", cfg);
    
    // 设置 PatternFormatterOptions
    quill::PatternFormatterOptions fmt_opts;
    fmt_opts.format_pattern = "%(message)";

    quill::Logger* logger = quill::Frontend::create_or_get_logger("recorder_logger", file_sink, fmt_opts);
    LOG_INFO(logger, "Type,Channel,Seq,Symbol,Price,Volume,Side/Bid,Type/Ask,Timestamp");

    uint64_t read_idx = 0;
    std::cout << "Recorder started (Quill MPMC). Writing to market_data.csv" << std::endl;

    while (true) {
        // 使用无锁等待获取数据
        const auto& msg = rb->wait_and_get(read_idx);
        
        if (msg.type == MsgType::Entrust) {
            const auto& o = msg.data.order;
            LOG_INFO(logger, "Order,{},{},{},{},{},{},{},{}", 
                o.channel_no, o.seq_no, o.symbol, o.price, o.volume, o.side, o.ord_type, o.timestamp);
        } else {
            const auto& e = msg.data.exec;
            LOG_INFO(logger, "Exec,{},{},{},{},{},{},{},{}", 
                e.channel_no, e.seq_no, e.symbol, e.price, e.volume, e.bid_no, e.ask_no, e.timestamp);
        }

        read_idx++;
        if (read_idx % 10000 == 0) {
            std::cout << "Processed " << read_idx << " messages..." << std::endl;
        }
    }

    return 0;
}

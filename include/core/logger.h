#pragma once

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/FileSink.h>
#include <string>

namespace etrader {
namespace core {

class Logger {
public:
    static void init(const std::string& filename, const std::string& level = "info") {
        quill::Backend::start();
        
        auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");
        auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(filename);
        
        quill::Logger* logger = quill::Frontend::create_or_get_logger("sys_logger", {console_sink, file_sink});
        
        if (level == "debug") logger->set_log_level(quill::LogLevel::Debug);
        else if (level == "info") logger->set_log_level(quill::LogLevel::Info);
        else if (level == "warn") logger->set_log_level(quill::LogLevel::Warning);
        else if (level == "error") logger->set_log_level(quill::LogLevel::Error);
    }

    static quill::Logger* get() {
        return quill::Frontend::get_logger("sys_logger");
    }
};

} // namespace core
} // namespace etrader

#define Q_LOG_DEBUG(fmt, ...) LOG_DEBUG(etrader::core::Logger::get(), fmt, ##__VA_ARGS__)
#define Q_LOG_INFO(fmt, ...) LOG_INFO(etrader::core::Logger::get(), fmt, ##__VA_ARGS__)
#define Q_LOG_WARN(fmt, ...) LOG_WARNING(etrader::core::Logger::get(), fmt, ##__VA_ARGS__)
#define Q_LOG_ERROR(fmt, ...) LOG_ERROR(etrader::core::Logger::get(), fmt, ##__VA_ARGS__)

#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

namespace etrader {
namespace core {

struct Config {
    std::string market_shm_name = "/market_journal";
    std::string strategy_shm_name = "/strategy_journal";
    std::string trade_shm_name = "/trade_journal";
    std::string log_level = "info";
    std::string log_file = "system.log";
    std::string strategy_so_path;
    nlohmann::json json_content;

    static Config load(const std::string& path) {
        Config cfg;
        std::ifstream f(path);
        if (f.is_open()) {
            nlohmann::json j;
            f >> j;
            cfg.json_content = j;
            if (j.contains("market_shm_name")) cfg.market_shm_name = j["market_shm_name"];
            if (j.contains("strategy_shm_name")) cfg.strategy_shm_name = j["strategy_shm_name"];
            if (j.contains("trade_shm_name")) cfg.trade_shm_name = j["trade_shm_name"];
            if (j.contains("log_level")) cfg.log_level = j["log_level"];
            if (j.contains("log_file")) cfg.log_file = j["log_file"];
            if (j.contains("strategy_so_path")) cfg.strategy_so_path = j["strategy_so_path"];
        }
        return cfg;
    }
};

} // namespace core
} // namespace etrader

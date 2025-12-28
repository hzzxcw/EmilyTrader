# EmilyTrader 🚀

EmilyTrader 是一个面向高性能、低延迟场景设计的中国 A 股行情接入与交易模拟系统。本项目旨在模拟真实的量化交易环境，采用工业级去中心化无锁架构，为策略开发和系统性能审计提供坚实基础。

## ✨ 核心特性

-   **去中心化架构**：借鉴 **KungFu (功夫) - YiJinJing** 引擎设计理念，采用单写多读（SPMC）的 Journal 通道模式。
-   **极致性能**：
    -   **无锁设计**：全链路数据交换无任何互斥锁（Mutex）或原子竞争。
    -   **缓存优化**：严格的缓存行对齐（Cache Line Alignment），消除伪共享。
    -   **微秒级响应**：下单全链路延迟稳定在 **1-3 微秒** 级别。
-   **全模块化设计**：
    -   `Simulator`：多线程高频行情模拟。
    -   `TradingEngine`：可插拔策略框架与 Mock 柜台。
    -   `EventLogger`：独立 Telemetry 进程，实时性能监控与审计。
-   **异步日志系统**：集成高性能 `Quill` 日志库，I/O 开销完全脱离热路径。

## 🏗️ 系统架构

项目的详细设计思想、目录结构及性能优化细节请参阅：[ARCHITECTURE.md](./ARCHITECTURE.md)

## 🚀 快速开始

### 依赖环境
-   **OS**: macOS 或 Linux
-   **Compiler**: 支持 C++17 的 Clang 或 GCC
-   **Build Tool**: [xmake](https://xmake.io/)

### 编译安装
```bash
# 克隆项目 (假设您已在 GitHub 上配置 SSH)
git clone git@github.com:hzzxcw/EmilyTrader.git
cd EmilyTrader

# 一键编译 (会自动下载 nlohmann_json 和 quill 依赖)
xmake
```

### 运行联调测试
```bash
# 赋予执行权限并运行
chmod +x run_all.sh
./run_all.sh
```

运行结束后，您可以查看生成的日志和统计数据：
-   `latency_stats.csv`：行情接入与下单延迟明细。
-   `trade_events.csv`：成交汇报与柜台响应记录。
-   `engine.log`：策略运行实时日志。

## 🛠️ 模块指南
-   **行情接入**：调用 `market::MarketWriter` 接口。
-   **策略开发**：继承 `BaseStrategy` 并实现 `on_tick` / `on_response`。
-   **配置修改**：编辑 `config/config.json` 调整 SHM 路径和日志级别。

---
*Created with ❤️ for Emily.*

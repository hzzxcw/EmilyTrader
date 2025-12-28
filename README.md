# EmilyTrader 🚀

EmilyTrader 是一个面向高性能、低延迟场景设计的中国 A 股行情接入与交易模拟系统。本项目旨在模拟真实的量化交易环境，采用工业级去中心化无锁架构，为策略开发和系统性能审计提供坚实基础。

## ✨ 核心特性

-   **去中心化架构**：借鉴 **KungFu (功夫) - YiJinJing** 引擎设计理念，采用单写多读（SPMC）的 Journal 通道模式。
-   **极致性能**：
    -   **无锁设计**：全链路数据交换无任何互斥锁（Mutex）或原子竞争。
    -   **缓存优化**：严格的缓存行对齐（Cache Line Alignment），消除伪共享。
    -   **微秒级响应**：下单全链路延迟稳定在 **1-3 微秒** 级别。
-   **分布式策略运行**：
    -   `Simulator`：多线程高频行情模拟（支持 Entrust/Execution 混合流）。
    -   `System Engine` (`apps/engine`)：系统主控与 Journal 管理器，负责环境初始化。
    -   `Exchange Service` (`apps/exchange`)：独立的 Mock 柜台服务，负责撮合与回报。
    -   `Strategy Runner` (`apps/strategy_loader`)：独立的策略宿主，支持动态加载 `.so` 策略库。
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
# 克隆项目
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
-   `market_data.csv`：录制的行情数据。
-   `exchange.log`：交易所仿真服务日志。
-   `strategy.log`：策略运行日志（含自定义参数）。
-   `strategy_stdout.log`：策略控制台输出（含 OrderBook 快照）。

## 🛠️ 模块指南
-   **策略开发**：
    1.  继承 `strategy::BaseStrategy`。
    2.  实现接口并在 `src/` 下创建 cpp 文件。
    3.  使用 `xmake` 编译为共享库 (`.so` / `.dylib`)。
    4.  在 `config/strategy.json` 中配置库路径和参数。
-   **配置修改**：
    -   `config/engine.json`: 系统主控配置。
    -   `config/exchange.json`: 交易所服务配置。
    -   `config/simulator.json`: 模拟器配置。
    -   `config/strategy.json`: 策略加载路径与参数。

---
*Created with ❤️ for Emily.*

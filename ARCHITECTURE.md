# EmilyTrader 系统架构文档

本项目是一个模拟中国 A 股行情接入与交易的高性能量化系统原型。其核心架构深受 **KungFu (功夫)** 的 **YiJinJing (易筋经)** 引擎启发，采用了**去中心化通道**、**全路径延迟审计**以及**分布式策略运行**的设计。

## 1. 系统目录结构

```text
.
├── config/                 # 配置文件目录
│   ├── engine.json         # 系统主控配置
│   ├── exchange.json       # 交易所仿真配置
│   ├── simulator.json      # 模拟器配置
│   ├── event_logger.json   # 审计与监控配置
│   └── strategy.json       # 策略加载配置 (含策略库路径与自定义参数)
├── include/
│   ├── core/               # 核心底层引擎
│   │   ├── common.h        # 基础类型定义与纳秒计时器
│   │   ├── frame.h         # 统一消息包装格式 (元数据头+Payload)
│   │   ├── journal.h       # 单写多读 (SPMC) 无锁通道实现
│   │   ├── poller.h        # 多路通道聚合轮询器
│   │   ├── config.h        # JSON 配置加载器
│   │   └── logger.h        # 基于 Quill 的异步日志封装
│   ├── market/             # 行情接入模块
│   │   └── market_writer.h # 高级写入 API (含 T1 自动打标)
│   ├── trade/              # 交易撮合/柜台模块
│   │   └── mock_trade.h    # 模拟柜台逻辑 (资金、持仓、回报)
│   ├── strategy/           # 策略框架模块
│   │   └── base_strategy.h # 策略基类定义
│   ├── orderbook/          # 订单簿模块
│   │   └── orderbook.h     # L2 订单簿构建与维护
│   └── event/              # 监控与审计模块
│       └── event_logger.h  # 全系统镜像落地 (CSV) 与延迟分析
├── apps/                   # 独立运行的进程应用
│   ├── simulator/          # 行情模拟程序
│   ├── engine/             # 系统主控 (Journal Manager)
│   ├── exchange/           # 交易所仿真服务 (Mock Exchange)
│   ├── strategy_loader/    # 策略加载运行器 (Strategy Runner)
│   └── event_logger/       # 数据落地与监控程序
├── src/
│   └── strategy/           # 策略源码 (编译为动态库)
│       └── simple_strategy.cpp
├── xmake.lua               # 现代化构建脚本
└── run_all.sh              # 一键联调启动脚本
```

---

## 2. 核心性能设计

### 2.1 去中心化与分布式 (Distributed & Decentralized)
系统采用了真正的分布式设计，各个组件通过共享内存（SHM）进行松耦合通信：

*   **System Engine (`apps/engine`)**：
    *   **角色**：系统主控与 Journal 管理器。
    *   **职责**：负责初始化和分配核心的共享内存文件（Journals），确保系统环境就绪。它不参与任何交易逻辑。
*   **Mock Exchange (`apps/exchange`)**：
    *   **角色**：交易所与柜台仿真服务。
    *   **职责**：作为 `MockTrade` 的宿主，监听 `Strategy Journal` 中的订单请求，进行模拟撮合，并将执行报告（ExecReport）写入 `Trade Journal`。
*   **Strategy Runner (`apps/strategy_loader`)**：
    *   **角色**：策略的宿主进程。
    *   **动态加载**：通过 `dlopen` 加载策略的共享库 (`.so`)，实现了策略逻辑与运行环境的隔离。
    *   **纯粹计算**：只负责接收行情、维护本地 OrderBook、发送指令。
*   **Simulator**：独立的行情源，负责向 `Market Journal` 写入高频行情（Entrust/Execution）。

### 2.2 订单簿 (OrderBook)
*   **功能**：基于逐笔委托（Entrust）和逐笔成交（Execution）构建实时的 L2 盘口（买一/卖一等）。
*   **位置**：策略内部维护一份私有的 `OrderBook` 对象，根据接收到的行情实时更新。
*   **特性**：支持快照输出（Snapshot），便于策略决策和调试。

### 2.3 动态策略加载
*   策略被编译为独立的共享对象 (`.so` / `.dylib`)。
*   `strategy_loader` 根据 `config/strategy.json` 配置动态加载策略库，并支持传入 JSON 配置参数（如风控阈值等）。

---

## 3. 全路径延迟审计 (Latency Telemetry)

系统定义了全链路的时间戳追踪体系：
*   **T0**：行情源产生时间（模拟外部数据源）。
*   **T1**：行情进入系统时间（MarketWriter 打标）。
*   **T2**：策略接收行情时间（Strategy 处理起点）。
*   **T3**：策略决策并写入指令通道的时间（Strategy 动作点）。

**EventLogger** 通过监听所有相关通道，实时计算并落地：
*   `Ingest_Lat (T1-T0)`：接入延迟。
*   `Order_Lat (T3-T1)`：策略内部决策耗时。

---

## 4. 模块化设计理念
*   **配置分离**：每个模块拥有独立的配置文件，支持自定义参数。
*   **接口解耦**：`StrategyContext` 不再持有 `MockTrade` 对象，而是纯粹作为 SHM 的写入接口，实现了策略与交易实现的物理隔离。

## 5. 如何扩展
1.  **开发新策略**：
    *   继承 `strategy::BaseStrategy`。
    *   实现 `on_init` (接收 JSON 配置), `on_tick`, `on_response`。
    *   在 cpp 中导出 `create_strategy` 工厂函数。
    *   编译为 `.so` 并在 `strategy.json` 中配置路径。
2.  **增加新行情源**：在 `apps/` 下新建程序，利用 `MarketWriter` 写入 `market_shm_name` 指定的通道。

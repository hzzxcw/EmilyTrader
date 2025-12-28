# EmilyTrader 系统架构文档

本项目是一个模拟中国 A 股行情接入与交易的高性能量化系统原型。其核心架构深受 **KungFu (功夫)** 的 **YiJinJing (易筋经)** 引擎启发，采用了**去中心化通道**和**全路径延迟审计**的设计。

## 1. 系统目录结构

```text
.
├── config/                 # 配置文件目录
│   └── config.json         # 定义 SHM 名称、日志级别、系统参数
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
│   │   ├── base_strategy.h # 策略基类定义
│   │   └── strategy_context.h # 策略运行上下文 (包含下单延迟审计)
│   └── event/              # 监控与审计模块
│       └── event_logger.h  # 全系统镜像落地 (CSV) 与延迟分析
├── apps/                   # 独立运行的进程应用
│   ├── simulator/          # 行情模拟程序
│   ├── engine/             # 策略执行主引擎
│   └── event_logger/       # 数据落地与监控程序
├── xmake.lua               # 现代化构建脚本
└── run_all.sh              # 一键联调启动脚本
```

---

## 2. 核心性能设计

### 2.1 去中心化通道 (Decentralized Journal)
系统放弃了传统的中心化 MPMC (多生产者多消费者) 队列，转而采用 **单写多读 (Single-Writer Multi-Reader)** 模式。
*   **零竞争写入**：每个通道（如行情通道、交易响应通道）仅允许一个特定的 Writer 进程写入。写操作仅涉及一个原子指针的递增，完全消除了 CPU 缓存行在多个核心间的剧烈波动（Cache Bouncing）。
*   **物理隔离**：不同的业务流存在于不同的共享内存段中。策略模块仅读取行情和回报，而不参与日志落地等重量级 I/O 竞争。

### 2.2 缓存行对齐 (Cache Line Alignment)
*   **防止伪共享**：所有的 `Frame` 结构体和 `Journal` 内部的 `write_idx` 均使用 `alignas(64)` 进行对齐。这确保了写索引和数据区不会落在同一个缓存行中，极大地提升了跨核读取的并行效率。

### 2.3 极速忙等 (Busy-Waiting)
*   系统在热路径上完全废弃了 `usleep` 或信号量等系统调用，采用 `cpu_pause()` (x86: `_mm_pause`, ARM: `yield`) 进行自旋。响应延迟从毫秒级直接降至**纳秒级**。

### 2.4 异步格式化日志 (Zero-Cost Formatting)
*   集成 **Quill** 日志库。与传统日志库不同，Quill 甚至将 `printf` 类字符串格式化的开销也从行情处理线程移到了后端线程。

---

## 3. 全路径延迟审计 (Latency Telemetry)

系统定义了全链路的时间戳追踪体系：
*   **T0**：行情源产生时间（模拟外部数据源）。
*   **T1**：行情进入系统时间（MarketWriter 打标）。
*   **T2**：策略接收行情时间（Strategy 处理起点）。
*   **T3**：决策下单并进入柜台时间（Order Action）。

**EventLogger** 通过 `Poller` 实时聚合上述所有数据，计算 `Ingest_Lat (T1-T0)` 和 `Order_Lat (T3-T1)`。通过 SHM 传输这些统计量，实现了对系统性能的**零干扰监控**。

---

## 4. 模块化设计理念
*   **可扩展性**：通过 `Frame` 结构的 `Payload` 模式，增加新的业务数据结构无需修改底层 SHM 逻辑，只需在 `data_types.h` 中定义即可。
*   **Mock 交易解耦**：`MockTrade` 被封装为独立的类，其状态变更通过 `TradeJournal` 异步广播。这使得策略可以像接入真实柜台一样，通过订阅消息来更新持仓，而不是同步阻塞等待结果。

## 5. 如何扩展
1.  **增加新行情源**：在 `apps/` 下新建程序，利用 `MarketWriter` 写入 `market_shm_name` 指定的通道。
2.  **开发新策略**：继承 `BaseStrategy` 并实现其虚函数，在 `TradingEngine` 中挂载即可。
3.  **增加监控指标**：修改 `LatencyStats` 结构并在策略逻辑中记录新时间戳。

# CTP 回测系统 (C++版本) - 架构设计

## 项目概述

一个基于 C++ 的高频交易回测系统，专门为中国期货市场设计，使用 CTP 接口。

## 核心架构

```
ctp_hft_backtest/
├── include/                    # 头文件
│   ├── core/                   # 核心引擎
│   │   ├── backtest_engine.h  # 回测引擎
│   │   ├── market_data.h      # 市场数据
│   │   ├── order_book.h       # 订单簿
│   │   └── position.h         # 持仓管理
│   ├── ctp/                    # CTP 接口
│   │   ├── ctp_gateway.h      # CTP 网关
│   │   ├── ctp_md_api.h       # 行情 API
│   │   └── ctp_td_api.h       # 交易 API
│   ├── strategy/               # 策略接口
│   │   ├── strategy.h         # 策略基类
│   │   └── indicators.h       # 技术指标
│   └── utils/                  # 工具类
│       ├── data_loader.h      # 数据加载
│       ├── performance.h      # 性能统计
│       └── logger.h           # 日志
├── src/                        # 源文件
│   ├── core/
│   ├── ctp/
│   ├── strategy/
│   └── utils/
├── examples/                   # 示例策略
│   ├── simple_mm/             # 简单做市策略
│   ├── grid_trading/          # 网格交易策略
│   └── momentum/              # 动量策略
├── data/                       # 数据目录
│   ├── raw/                   # 原始 CTP 数据
│   └── processed/             # 处理后的数据
├── build/                      # 编译输出
├── tests/                      # 测试代码
└── docs/                       # 文档
```

## 核心模块设计

### 1. 回测引擎 (BacktestEngine)

```cpp
class BacktestEngine {
public:
    // 初始化
    void Initialize(const BacktestConfig& config);

    // 加载数据
    void LoadData(const std::string& data_file);

    // 运行策略
    void Run(Strategy* strategy);

    // 获取结果
    PerformanceReport GetReport() const;

private:
    // 订单簿管理
    OrderBook order_book_;

    // 持仓管理
    PositionManager position_manager_;

    // 订单管理
    OrderManager order_manager_;

    // 市场数据
    std::vector<MarketData> market_data_;

    // 当前时间
    int64_t current_time_;
};
```

### 2. 订单簿 (OrderBook)

```cpp
class OrderBook {
public:
    // 更新市场数据
    void Update(const MarketData& data);

    // 获取最优买价
    double GetBestBid() const;

    // 获取最优卖价
    double GetBestAsk() const;

    // 获取中间价
    double GetMidPrice() const;

    // 获取深度
    std::vector<DepthLevel> GetBids(int depth) const;
    std::vector<DepthLevel> GetAsks(int depth) const;

private:
    std::map<double, double> bids_;  // 价格->数量
    std::map<double, double> asks_;
    double tick_size_;
    double lot_size_;
};
```

### 3. 策略接口 (Strategy)

```cpp
class Strategy {
public:
    virtual ~Strategy() = default;

    // 策略初始化
    virtual void OnInit(BacktestEngine* engine) = 0;

    // 市场数据回调
    virtual void OnMarketData(const MarketData& data) = 0;

    // 订单成交回调
    virtual void OnOrderFilled(const Order& order) = 0;

    // 策略结束
    virtual void OnStop() = 0;

protected:
    // 下单接口
    void SubmitOrder(const Order& order);
    void CancelOrder(int order_id);

    // 获取订单簿
    const OrderBook& GetOrderBook() const;

    // 获取持仓
    const Position& GetPosition() const;
};
```

### 4. CTP 网关 (CtpGateway)

```cpp
class CtpGateway {
public:
    // 连接服务器
    bool Connect(const CtpConfig& config);

    // 订阅行情
    bool SubscribeMarketData(const std::string& instrument_id);

    // 下单
    int SubmitOrder(const OrderRequest& request);

    // 撤单
    bool CancelOrder(int order_id);

    // 查询账户
    Account QueryAccount();

    // 查询持仓
    std::vector<Position> QueryPositions();

private:
    CThostFtdcTraderApi* trader_api_;
    CThostFtdcMdApi* md_api_;
    CtpTraderSpi trader_spi_;
    CtpMdSpi md_spi_;
};
```

## 数据格式

### 市场数据格式

```cpp
struct MarketData {
    int64_t timestamp;           // 时间戳 (纳秒)
    std::string instrument_id;   // 合约代码
    double last_price;           // 最新价
    double bid_price;            // 买一价
    double ask_price;            // 卖一价
    double bid_volume;           // 买一量
    double ask_volume;           // 卖一量
    double volume;               // 成交量
    double open_interest;        // 持仓量
};
```

### 订单格式

```cpp
struct Order {
    int order_id;                // 订单ID
    std::string instrument_id;   // 合约代码
    OrderSide side;              // 买卖方向
    OrderType type;              // 订单类型
    double price;                // 价格
    double quantity;             // 数量
    OrderStatus status;          // 订单状态
    double filled_qty;           // 成交数量
    double avg_price;            // 成交均价
};
```

## 策略开发流程

### 1. 继承 Strategy 类

```cpp
class MyStrategy : public Strategy {
public:
    void OnInit(BacktestEngine* engine) override {
        // 初始化策略参数
        tick_size_ = engine->GetTickSize();
        lot_size_ = engine->GetLotSize();
    }

    void OnMarketData(const MarketData& data) override {
        // 处理市场数据
        auto mid_price = GetOrderBook().GetMidPrice();

        // 策略逻辑
        if (ShouldBuy(mid_price)) {
            SubmitBuyOrder(mid_price - 1.0, 10);
        }
        if (ShouldSell(mid_price)) {
            SubmitSellOrder(mid_price + 1.0, 10);
        }
    }

    void OnOrderFilled(const Order& order) override {
        // 处理订单成交
        std::cout << "订单成交: " << order.order_id << std::endl;
    }

    void OnStop() override {
        // 策略结束
        std::cout << "策略结束" << std::endl;
    }

private:
    double tick_size_;
    double lot_size_;
};
```

### 2. 配置回测参数

```cpp
BacktestConfig config;
config.data_file = "data/ctp_rb2501_20240801.npz";
config.tick_size = 1.0;
config.lot_size = 10.0;
config.initial_capital = 1000000.0;
config.commission_rate = 0.0001;
```

### 3. 运行回测

```cpp
int main() {
    // 创建回测引擎
    BacktestEngine engine;
    engine.Initialize(config);

    // 创建策略
    MyStrategy strategy;

    // 运行回测
    engine.Run(&strategy);

    // 获取结果
    auto report = engine.GetReport();
    report.Print();

    return 0;
}
```

## 编译和运行

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

### 运行示例

```bash
# 运行简单做市策略
./simple_market_making

# 运行网格交易策略
./grid_trading

# 运行动量策略
./momentum_strategy
```

## 特性

### 核心特性

- ✅ 纯 C++ 实现，无第三方依赖
- ✅ 支持 CTP 接口
- ✅ 高性能回测引擎
- ✅ 完整的订单簿模拟
- ✅ 支持多种订单类型
- ✅ 真实的滑点和成交模拟
- ✅ 详细的性能统计

### 策略特性

- ✅ 简单易用的策略接口
- ✅ 支持多种技术指标
- ✅ 灵活的风险管理
- ✅ 支持多合约策略
- ✅ 支持套利策略

### 数据特性

- ✅ 支持 CTP 原始数据
- ✅ 支持自定义数据格式
- ✅ 自动数据转换
- ✅ 数据验证和清洗

## 性能优化

- 使用高效的数据结构
- 内存管理优化
- 并行处理支持
- JIT 编译（可选）

## 扩展性

- 插件化架构
- 自定义指标
- 自定义事件
- 多策略组合

## 文档

- API 文档
- 策略开发指南
- 数据格式说明
- 性能优化指南

---

这个架构设计提供了一个清晰、易用的 C++ 回测系统，专注于 CTP 接口和中国期货市场。

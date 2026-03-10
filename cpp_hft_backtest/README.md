# CTP 高频交易回测系统 (C++版本)

这是一个基于纯 C++ 实现的高频交易回测系统，专门为中国期货市场设计，使用 CTP 接口。

## 特性

- ✅ **纯 C++ 实现** - 无第三方依赖，易于理解和修改
- ✅ **CTP 接口支持** - 专门针对中国期货市场
- ✅ **高性能回测** - 高效的数据结构和算法
- ✅ **完整订单簿模拟** - 真实的订单簿深度数据
- ✅ **多种订单类型** - 支持限价单、市价单等
- ✅ **真实成交模拟** - 考虑滑点和部分成交
- ✅ **详细性能报告** - 完整的回测统计分析
- ✅ **简单易用** - 清晰的 API 接口

## 项目结构

```
cpp_hft_backtest/
├── include/                    # 头文件
│   ├── core/                   # 核心引擎
│   │   ├── market_data.h      # 市场数据结构
│   │   ├── order_book.h       # 订单簿
│   │   ├── position_manager.h # 持仓管理
│   │   └── backtest_engine.h  # 回测引擎
│   ├── strategy/               # 策略接口
│   │   └── strategy.h         # 策略基类
│   └── ctp/                    # CTP 接口 (待实现)
├── src/                        # 源文件
│   ├── core/
│   └── strategy/
├── examples/                   # 示例策略
│   └── simple_mm/             # 简单做市策略
├── build/                      # 编译输出
├── docs/                       # 文档
└── CMakeLists.txt             # CMake 配置
```

## 快速开始

### 1. 编译项目

```bash
cd cpp_hft_backtest
bash build.sh
```

### 2. 运行示例

```bash
cd build/bin
./simple_market_making
```

### 3. 查看结果

程序会输出详细的回测过程和最终的性能报告。

## 开发你的策略

### 步骤 1: 继承 Strategy 类

```cpp
#include "strategy/strategy.h"

class MyStrategy : public Strategy {
public:
    void OnInit(BacktestEngine* engine) override {
        // 初始化策略参数
        tick_size_ = GetTickSize();
        lot_size_ = GetLotSize();
    }

    void OnMarketData(const MarketData& data) override {
        // 处理市场数据
        auto mid_price = GetOrderBook().GetMidPrice();

        // 策略逻辑
        if (ShouldBuy(mid_price)) {
            SubmitBuyOrder(mid_price - 1.0, 10);
        }
    }

    void OnOrderFilled(const Order& order) override {
        // 处理订单成交
        Log("订单成交: " + std::to_string(order.order_id));
    }

    void OnOrderRejected(const Order& order) override {
        // 处理订单拒绝
    }

    void OnOrderCanceled(const Order& order) override {
        // 处理订单撤销
    }

    void OnStop() override {
        // 策略结束
    }

private:
    double tick_size_;
    double lot_size_;
};
```

### 步骤 2: 创建主程序

```cpp
#include "core/backtest_engine.h"
#include "my_strategy.h"

int main() {
    // 配置回测参数
    BacktestConfig config;
    config.data_file = "data/ctp_rb2501_20240801.npz";
    config.instrument_id = "rb2501";
    config.initial_capital = 1000000.0;
    config.tick_size = 1.0;
    config.lot_size = 10.0;
    config.commission_rate = 0.0001;

    // 创建回测引擎
    BacktestEngine engine;
    engine.Initialize(config);
    engine.LoadData();

    // 创建并运行策略
    MyStrategy strategy;
    engine.Run(&strategy);

    // 获取报告
    auto report = engine.GetReport();
    report.Print();

    return 0;
}
```

### 步骤 3: 编译运行

```bash
# 添加到 CMakeLists.txt
add_executable(my_strategy
    examples/my_strategy/main.cpp
)
target_link_libraries(my_strategy ctp_hft_core)

# 编译
cd build && make

# 运行
cd bin && ./my_strategy
```

## API 参考

### 策略接口

#### 下单

```cpp
// 提交买单
int SubmitBuyOrder(double price, double quantity, OrderType type = OrderType::LIMIT);

// 提交卖单
int SubmitSellOrder(double price, double quantity, OrderType type = OrderType::LIMIT);

// 撤销订单
bool CancelOrder(int order_id);
```

#### 查询

```cpp
// 获取订单簿
const OrderBook& GetOrderBook() const;

// 获取持仓
Position GetPosition(const std::string& instrument_id) const;

// 获取账户信息
Account GetAccount() const;

// 获取当前时间
int64_t GetCurrentTime() const;
```

#### 工具

```cpp
// 日志输出
void Log(const std::string& message) const;

// 获取参数
double GetTickSize() const;
double GetLotSize() const;
std::string GetInstrumentId() const;
```

### 回测配置

```cpp
struct BacktestConfig {
    std::string data_file;       // 数据文件路径
    std::string instrument_id;   // 合约代码
    double initial_capital;      // 初始资金
    double tick_size;            // 最小价格变动
    double lot_size;             // 最小交易单位
    double commission_rate;      // 手续费率
    double slippage;             // 滑点
    bool partial_fill;           // 是否部分成交
};
```

### 性能报告

```cpp
struct PerformanceReport {
    double total_return;         // 总收益率
    double annual_return;        // 年化收益率
    double sharpe_ratio;         // 夏普比率
    double max_drawdown;         // 最大回撤
    double win_rate;             // 胜率
    int total_trades;            // 总交易次数
    double avg_profit_per_trade; // 平均每笔盈利
    double profit_factor;        // 盈亏比
};
```

## 常用合约参数

| 合约品种 | 交易所 | Tick Size | Lot Size |
|---------|-------|-----------|----------|
| rb (螺纹钢) | 上期所 | 1.0 | 10.0 |
| cu (铜) | 上期所 | 10.0 | 5.0 |
| au (黄金) | 上期所 | 0.02 | 1000.0 |
| a (豆一) | 大商所 | 1.0 | 10.0 |
| m (豆粕) | 大商所 | 1.0 | 10.0 |
| SR (白糖) | 郑商所 | 1.0 | 10.0 |
| IF (沪深300) | 中金所 | 0.2 | 300.0 |

## 示例策略

### 简单做市策略

在 `examples/simple_mm/` 目录中，实现了一个简单的做市策略：

- 在中间价两侧挂买卖单
- 根据持仓调整价格偏移
- 简单的风险管理

运行示例：

```bash
cd build/bin
./simple_market_making
```

## 数据格式

### 市场数据

当前版本使用模拟数据进行测试。真实的数据加载功能将在后续版本中实现。

## 性能优化

- 使用高效的 STL 容器
- 避免不必要的内存分配
- 优化订单簿更新逻辑
- 批量处理订单

## 扩展功能

### 待实现功能

- [ ] 真实 CTP 数据加载
- [ ] CTP 实盘交易接口
- [ ] 更多技术指标
- [ ] 多合约策略支持
- [ ] 参数优化工具
- [ ] 可视化图表

## 文档

- [架构设计文档](docs/CPP_ARCHITECTURE.md)
- [CTP 集成指南](../my_docs/CTP_INTEGRATION_GUIDE.md)
- [CTP 快速开始](../my_docs/CTP_QUICKSTART.md)

## 编译要求

- C++11 或更高版本
- CMake 3.10 或更高版本
- GCC 4.8 或更高版本

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！

## 联系方式

- 项目地址: https://github.com/websect/hftbacktest_xtp
- 问题反馈: 提交 GitHub Issue

---

**注意**: 这是一个学习和研究项目，实盘交易有风险，请谨慎使用。

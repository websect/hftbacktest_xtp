# CTP 连接器集成指南

本文档详细说明如何使用 CTP (China Futures Trading Protocol) 连接器进行中国期货市场的回测和实盘交易。

## 目录

1. [概述](#概述)
2. [架构设计](#架构设计)
3. [安装配置](#安装配置)
4. [数据准备](#数据准备)
5. [回测示例](#回测示例)
6. [实盘交易](#实盘交易)
7. [API 参考](#api-参考)
8. [故障排除](#故障排除)

## 概述

### 什么是 CTP？

CTP (China Futures Trading Protocol) 是中国期货市场的主要交易接口，由上海期货信息技术有限公司开发。它支持上海期货交易所、大连商品交易所、郑州商品交易所和中国金融期货交易所的所有合约。

### CTP 连接器特性

- ✅ 支持国内所有期货交易所
- ✅ Level-2 行情数据
- ✅ 实时订单管理
- ✅ 完整的账户管理
- ✅ 高性能异步处理
- ✅ 自动重连机制

### 适用场景

- 国内期货策略回测
- 程序化交易
- 套利交易
- 高频交易

## 架构设计

### 系统架构

```
┌─────────────┐
│   策略层     │
│  (Python)   │
└──────┬──────┘
       │
┌──────▼──────┐
│ hftbacktest │
│   核心引擎   │
└──────┬──────┘
       │
┌──────▼──────┐
│ CTP 连接器   │
│   (Rust)    │
└──────┬──────┘
       │
┌──────▼──────┐
│  CTP SDK    │
│  (C++)      │
└──────┬──────┘
       │
┌──────▼──────┐
│  期货交易所   │
└─────────────┘
```

### 模块结构

```
connector/src/ctp/
├── mod.rs                    # 主连接器实现
├── market_data_stream.rs     # 行情数据流
├── order_manager.rs          # 订单管理器
├── rest.rs                   # REST API 客户端
├── user_data_stream.rs       # 用户数据流
└── msg.rs                    # 消息定义
```

## 安装配置

### 1. 环境要求

- Python 3.11+
- Rust 1.90+
- CTP SDK (需要从期货公司获取)

### 2. 安装依赖

```bash
# 安装 hftbacktest
pip install hftbacktest

# 安装 CTP Python 绑定（可选）
pip install vnpy-ctp
```

### 3. 配置 CTP 连接器

创建配置文件 `ctp_config.toml`:

```toml
[ctp]
# 期货公司配置
broker_id = "9999"           # 期货公司 ID
investor_id = "your_id"      # 投资者 ID
password = "your_password"   # 密码
app_id = "your_app_id"       # 应用 ID
auth_code = "your_auth_code" # 认证码

# 前置服务器地址
front_md = "tcp://180.168.146.187:10131"  # 行情前置
front_td = "tcp://180.168.146.187:10130"  # 交易前置

# 订单配置
order_prefix = "ctp_xtp"     # 订单前缀
```

### 4. 常用前置服务器

#### 上海期货交易所

```
行情前置: tcp://180.168.146.187:10131
交易前置: tcp://180.168.146.187:10130
```

#### 大连商品交易所

```
行情前置: tcp://180.168.146.187:10141
交易前置: tcp://180.168.146.187:10140
```

#### 郑州商品交易所

```
行情前置: tcp://180.168.146.187:10151
交易前置: tcp://180.168.146.187:10150
```

#### 中国金融期货交易所

```
行情前置: tcp://180.168.146.187:10121
交易前置: tcp://180.168.146.187:10120
```

## 数据准备

### 1. 数据格式要求

hftbacktest 需要以下格式的数据：

- **时间戳**: 纳秒级 Unix 时间戳
- **买卖价**: 以 tick 为单位的价格
- **买卖量**: 以手为单位的数量
- **成交数据**: 成交价格和数量

### 2. 使用数据转换工具

```python
from my_extensions.ctp_data_converter import create_sample_data

# 创建示例数据
create_sample_data('data/ctp_rb2501_20240801.npz', num_records=10000)
```

### 3. 转换实际 CTP 数据

```python
from my_extensions.ctp_data_converter import convert_ctp_file

# 转换 CTP CSV 数据
convert_ctp_file(
    input_file='path/to/ctp_data.csv',
    output_file='data/ctp_rb2501_20240801.npz',
    tick_size=1.0,
    lot_size=10.0
)
```

### 4. 常用合约参数

| 合约品种 | 交易所 | Tick Size | Lot Size |
|---------|-------|-----------|----------|
| rb (螺纹钢) | 上期所 | 1.0 | 10.0 |
| cu (铜) | 上期所 | 10.0 | 5.0 |
| au (黄金) | 上期所 | 0.02 | 1000.0 |
| a (豆一) | 大商所 | 1.0 | 10.0 |
| m (豆粕) | 大商所 | 1.0 | 10.0 |
| SR (白糖) | 郑商所 | 1.0 | 10.0 |
| IF (沪深300) | 中金所 | 0.2 | 300.0 |

## 回测示例

### 1. 基础回测

```python
import numpy as np
from numba import njit
from hftbacktest import BacktestAsset, HashMapMarketDepthBacktest, BUY, SELL, GTX, LIMIT

@njit
def my_strategy(hbt):
    asset_no = 0
    tick_size = hbt.depth(asset_no).tick_size
    lot_size = hbt.depth(asset_no).lot_size

    while hbt.elapse(10_000_000) == 0:
        hbt.clear_inactive_orders(asset_no)

        # 获取市场深度
        depth = hbt.depth(asset_no)
        mid_price = (depth.best_bid + depth.best_ask) / 2.0

        # 策略逻辑
        # ... 你的策略代码 ...

        # 提交订单
        hbt.submit_buy_order(asset_no, 1, mid_price - 1.0, 1.0, GTX, LIMIT, False)
        hbt.submit_sell_order(asset_no, 2, mid_price + 1.0, 1.0, GTX, LIMIT, False)

        # 等待响应
        hbt.wait_order_response(asset_no, 2, 5_000_000_000)

    return True

# 配置回测
asset = (
    BacktestAsset()
    .data(['data/ctp_rb2501_20240801.npz'])
    .initial_snapshot('data/ctp_rb2501_20240731_eod.npz')
    .linear_asset(1.0)
    .intp_order_latency(['latency/ctp_order_latency_20240801.npz'])
    .power_prob_queue_model(2.0)
    .no_partial_fill_exchange()
    .trading_value_fee_model(0.0001, 0.0001)
    .tick_size(1.0)
    .lot_size(10.0)
)

hbt = HashMapMarketDepthBacktest([asset])
result = my_strategy(hbt)
```

### 2. 运行示例

```bash
# 创建示例数据
python my_extensions/ctp_data_converter.py

# 运行回测
python my_extensions/example_ctp.py
```

### 3. 查看结果

```python
from hftbacktest.stats import compute_stats

# 计算统计指标
stats = compute_stats(hbt)

print(f"总收益: {stats['total_return']:.2%}")
print(f"夏普比率: {stats['sharpe_ratio']:.2f}")
print(f"最大回撤: {stats['max_drawdown']:.2%}")
```

## 实盘交易

### 1. 启动 Connector

```bash
# 构建 CTP 连接器
cd hftbacktest/connector
cargo build --release

# 启动连接器
./target/release/connector \
  --name ctp \
  --connector ctp \
  --config ctp_config.toml
```

### 2. 连接实盘

```python
from hftbacktest.connector import CtpConnector

# 创建连接器
connector = CtpConnector('ctp_config.toml')

# 注册合约
connector.register('rb2501')  # 螺纹钢主力合约

# 启动连接
connector.run()

# 运行策略
# ... 策略代码 ...
```

### 3. 风险控制

```python
# 设置仓位限制
max_position = 100  # 最大持仓手数
max_loss_per_day = 10000  # 每日最大亏损

# 实时监控
position = hbt.position(asset_no)
notional_value = position * current_price

if abs(notional_value) > max_position * lot_size * current_price:
    # 平仓
    hbt.close_position(asset_no)
```

## API 参考

### CtpConnector

#### 初始化

```python
connector = CtpConnector(config_file)
```

#### 注册合约

```python
connector.register(symbol)
```

#### 提交订单

```python
connector.submit_order(
    symbol='rb2501',
    side=BUY,
    price=3500.0,
    qty=10,
    order_type=LIMIT,
    time_in_force=GTC
)
```

#### 撤销订单

```python
connector.cancel_order(order_id)
```

#### 查询订单

```python
order = connector.query_order(order_id)
```

### 数据类型

#### OrderResponse

```python
{
    'order_id': 'ctp_xtp_rb2501_123',
    'status': 1,  # 0: 全部成交, 1: 部分成交, 3: 已撤单
    'filled_qty': 5.0,
    'avg_price': 3500.0
}
```

#### MarketData

```python
{
    'symbol': 'rb2501',
    'last_price': 3500.0,
    'bid_price': 3499.0,
    'ask_price': 3501.0,
    'bid_qty': 100,
    'ask_qty': 80,
    'volume': 100000,
    'open_interest': 50000
}
```

## 故障排除

### 常见问题

#### 1. 连接失败

**问题**: 无法连接到 CTP 前置服务器

**解决方案**:
- 检查网络连接
- 确认前置服务器地址正确
- 检查防火墙设置
- 联系期货公司确认服务器状态

#### 2. 认证失败

**问题**: 登录认证失败

**解决方案**:
- 检查账号密码是否正确
- 确认投资者 ID 和密码
- 检查应用 ID 和认证码
- 联系期货公司重置密码

#### 3. 数据格式错误

**问题**: 数据转换失败

**解决方案**:
- 检查数据文件格式
- 确认 tick_size 和 lot_size 参数
- 查看错误日志获取详细信息

#### 4. 订单拒绝

**问题**: 订单被交易所拒绝

**解决方案**:
- 检查订单参数是否合法
- 确认资金和持仓充足
- 检查交易时间是否正确
- 查看交易所返回的错误信息

### 日志调试

```python
import logging

# 启用详细日志
logging.basicConfig(level=logging.DEBUG)

# 查看 CTP 连接器日志
connector.set_log_level('DEBUG')
```

### 性能优化

```python
# 使用 Numba 加速策略
@njit
def optimized_strategy(hbt):
    # 策略代码
    pass

# 批量处理订单
orders = []
for i in range(10):
    orders.append({
        'symbol': 'rb2501',
        'side': BUY,
        'price': 3500.0 + i,
        'qty': 10
    })
connector.submit_batch_orders(orders)
```

## 最佳实践

### 1. 数据管理

- 定期备份历史数据
- 使用压缩格式存储
- 建立数据质量检查机制

### 2. 风险控制

- 设置止损止盈
- 限制单日最大亏损
- 监控账户资金使用率

### 3. 策略开发

- 从简单策略开始
- 充分回测验证
- 模拟盘测试后再实盘

### 4. 系统监控

- 监控连接状态
- 记录所有交易记录
- 设置异常报警

## 进阶主题

### 1. 多合约策略

```python
# 注册多个合约
for symbol in ['rb2501', 'hc2501', 'i2501']:
    connector.register(symbol)
```

### 2. 跨市场套利

```python
# 跨品种套利
rb_price = get_market_depth('rb2501').mid_price
hc_price = get_market_depth('hc2501').mid_price

spread = rb_price - hc_price
if spread > threshold:
    # 执行套利策略
    pass
```

### 3. 高频交易优化

```python
# 使用低延迟数据结构
from hftbacktest import VectorMarketDepth

depth = VectorMarketDepth(tick_size, lot_size)

# 批量处理订单
batch_submit(orders)
```

## 参考资源

- [CTP 官方文档](http://www.sfit.com.cn/)
- [hftbacktest 文档](https://hftbacktest.readthedocs.io/)
- [上海期货交易所](http://www.shfe.com.cn/)
- [大连商品交易所](http://www.dce.com.cn/)
- [郑州商品交易所](http://www.czce.com.cn/)
- [中国金融期货交易所](http://www.cffex.com.cn/)

## 联系支持

- 项目地址: https://github.com/websect/hftbacktest_xtp
- 问题反馈: 提交 GitHub Issue
- 技术讨论: 项目 Discussions

---

**最后更新**: 2026-03-10
**版本**: 1.0.0

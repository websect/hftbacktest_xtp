#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <string>
#include <cstdint>
#include <vector>

namespace ctp_hft {

/// 订单方向
enum class OrderSide {
    BUY = 0,    // 买入
    SELL = 1    // 卖出
};

/// 订单类型
enum class OrderType {
    LIMIT = 0,      // 限价单
    MARKET = 1,     // 市价单
    STOP = 2,       // 止损单
    IOC = 3,        // 立即成交否则撤销
    FOK = 4,        // 全部成交否则撤销
    GTX = 5         // 只能做流动性提供者
};

/// 订单状态
enum class OrderStatus {
    NEW = 0,            // 已提交
    PARTIAL_FILLED = 1, // 部分成交
    FILLED = 2,         // 全部成交
    CANCELED = 3,       // 已撤销
    REJECTED = 4,       // 已拒绝
    EXPIRED = 5         // 已过期
};

/// 深度数据
struct DepthLevel {
    double price;
    double volume;

    DepthLevel(double p = 0.0, double v = 0.0) : price(p), volume(v) {}
};

/// 市场数据
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

    std::vector<DepthLevel> bids;  // 买盘深度
    std::vector<DepthLevel> asks;  // 卖盘深度

    MarketData()
        : timestamp(0)
        , last_price(0.0)
        , bid_price(0.0)
        , ask_price(0.0)
        , bid_volume(0.0)
        , ask_volume(0.0)
        , volume(0.0)
        , open_interest(0.0) {}
};

/// 订单
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
    int64_t create_time;         // 创建时间
    int64_t update_time;         // 更新时间

    Order()
        : order_id(0)
        , side(OrderSide::BUY)
        , type(OrderType::LIMIT)
        , price(0.0)
        , quantity(0.0)
        , status(OrderStatus::NEW)
        , filled_qty(0.0)
        , avg_price(0.0)
        , create_time(0)
        , update_time(0) {}
};

/// 订单请求
struct OrderRequest {
    std::string instrument_id;   // 合约代码
    OrderSide side;              // 买卖方向
    OrderType type;              // 订单类型
    double price;                // 价格
    double quantity;             // 数量

    OrderRequest()
        : side(OrderSide::BUY)
        , type(OrderType::LIMIT)
        , price(0.0)
        , quantity(0.0) {}
};

/// 持仓信息
struct Position {
    std::string instrument_id;   // 合约代码
    double long_position;        // 多头持仓
    double short_position;       // 空头持仓
    double long_available;       // 多头可用
    double short_available;      // 空头可用
    double long_avg_price;       // 多头均价
    double short_avg_price;      // 空头均价
    double unrealized_pnl;       // 浮动盈亏

    Position()
        : long_position(0.0)
        , short_position(0.0)
        , long_available(0.0)
        , short_available(0.0)
        , long_avg_price(0.0)
        , short_avg_price(0.0)
        , unrealized_pnl(0.0) {}
};

/// 账户信息
struct Account {
    std::string account_id;      // 账户ID
    double balance;              // 总资金
    double available;            // 可用资金
    double margin;               // 保证金
    double frozen_margin;        // 冻结保证金
    double commission;           // 手续费
    double position_pnl;         // 持仓盈亏
    double close_pnl;            // 平仓盈亏

    Account()
        : balance(0.0)
        , available(0.0)
        , margin(0.0)
        , frozen_margin(0.0)
        , commission(0.0)
        , position_pnl(0.0)
        , close_pnl(0.0) {}
};

} // namespace ctp_hft

#endif // MARKET_DATA_H

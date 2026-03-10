#ifndef STRATEGY_H
#define STRATEGY_H

#include "core/market_data.h"
#include "core/order_book.h"
#include "core/position_manager.h"
#include <string>

namespace ctp_hft {

/// 回测引擎前置声明
class BacktestEngine;

/// 策略基类
class Strategy {
public:
    virtual ~Strategy() = default;

    /// 策略初始化
    virtual void OnInit(BacktestEngine* engine) = 0;

    /// 市场数据回调
    virtual void OnMarketData(const MarketData& data) = 0;

    /// 订单成交回调
    virtual void OnOrderFilled(const Order& order) = 0;

    /// 订单拒绝回调
    virtual void OnOrderRejected(const Order& order) = 0;

    /// 订单撤销回调
    virtual void OnOrderCanceled(const Order& order) = 0;

    /// 策略结束
    virtual void OnStop() = 0;

protected:
    /// 提交买单
    int SubmitBuyOrder(double price, double quantity, OrderType type = OrderType::LIMIT);

    /// 提交卖单
    int SubmitSellOrder(double price, double quantity, OrderType type = OrderType::LIMIT);

    /// 撤销订单
    bool CancelOrder(int order_id);

    /// 获取订单簿
    const OrderBook& GetOrderBook() const;

    /// 获取持仓
    Position GetPosition(const std::string& instrument_id) const;

    /// 获取账户信息
    Account GetAccount() const;

    /// 获取当前时间
    int64_t GetCurrentTime() const;

    /// 获取合约代码
    std::string GetInstrumentId() const;

    /// 获取 tick size
    double GetTickSize() const;

    /// 获取 lot size
    double GetLotSize() const;

    /// 日志输出
    void Log(const std::string& message) const;

private:
    BacktestEngine* engine_;
    friend class BacktestEngine;
};

} // namespace ctp_hft

#endif // STRATEGY_H

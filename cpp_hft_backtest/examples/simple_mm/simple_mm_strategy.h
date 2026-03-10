#include "strategy/strategy.h"
#include <iostream>
#include <cmath>

namespace ctp_hft {

/// 简单做市策略
class SimpleMarketMakingStrategy : public Strategy {
public:
    SimpleMarketMakingStrategy()
        : tick_size_(1.0)
        , lot_size_(1.0)
        , half_spread_(2.0)
        , order_qty_(10.0) {
    }

    void OnInit(BacktestEngine* engine) override {
        Log("策略初始化");

        tick_size_ = GetTickSize();
        lot_size_ = GetLotSize();

        Log("Tick Size: " + std::to_string(tick_size_));
        Log("Lot Size: " + std::to_string(lot_size_));
    }

    void OnMarketData(const MarketData& data) override {
        // 获取订单簿
        const OrderBook& order_book = GetOrderBook();

        // 获取中间价
        double mid_price = order_book.GetMidPrice();
        if (mid_price <= 0) {
            return;
        }

        // 获取当前持仓
        Position pos = GetPosition(data.instrument_id);
        double net_position = pos.long_position - pos.short_position;

        // 计算保留价格（考虑风险）
        double risk_skew = -0.5 * net_position;  // 风险偏斜
        double reservation_price = mid_price + risk_skew;

        // 计算买卖价
        double new_bid = reservation_price - half_spread_;
        double new_ask = reservation_price + half_spread_;

        // 对齐到 tick
        new_bid = std::round(new_bid / tick_size_) * tick_size_;
        new_ask = std::round(new_ask / tick_size_) * tick_size_;

        Log("中间价: " + std::to_string(mid_price) +
            ", 买价: " + std::to_string(new_bid) +
            ", 卖价: " + std::to_string(new_ask));

        // 这里应该管理现有订单，简化处理直接下单
        // 实际应用中需要更复杂的订单管理逻辑

        // 提交买单
        SubmitBuyOrder(new_bid, order_qty_);

        // 提交卖单
        SubmitSellOrder(new_ask, order_qty_);
    }

    void OnOrderFilled(const Order& order) override {
        Log("订单成交: ID=" + std::to_string(order.order_id) +
            ", 价格=" + std::to_string(order.avg_price) +
            ", 数量=" + std::to_string(order.filled_qty) +
            ", 方向=" + (order.side == OrderSide::BUY ? "买入" : "卖出"));

        // 可以在这里添加成交后的处理逻辑
    }

    void OnOrderRejected(const Order& order) override {
        Log("订单拒绝: ID=" + std::to_string(order.order_id));
    }

    void OnOrderCanceled(const Order& order) override {
        Log("订单撤销: ID=" + std::to_string(order.order_id));
    }

    void OnStop() override {
        Log("策略结束");

        // 输出最终持仓
        Position pos = GetPosition(GetInstrumentId());
        Log("最终多头持仓: " + std::to_string(pos.long_position));
        Log("最终空头持仓: " + std::to_string(pos.short_position));
        Log("浮动盈亏: " + std::to_string(pos.unrealized_pnl));
    }

private:
    double tick_size_;
    double lot_size_;
    double half_spread_;
    double order_qty_;
};

} // namespace ctp_hft

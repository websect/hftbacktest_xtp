#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "market_data.h"
#include <map>
#include <vector>
#include <string>

namespace ctp_hft {

/// 订单簿类
class OrderBook {
public:
    OrderBook(double tick_size = 1.0, double lot_size = 1.0);

    /// 更新市场数据
    void Update(const MarketData& data);

    /// 获取最优买价
    double GetBestBid() const;

    /// 获取最优卖价
    double GetBestAsk() const;

    /// 获取中间价
    double GetMidPrice() const;

    /// 获取买盘深度
    std::vector<DepthLevel> GetBids(int depth = 5) const;

    /// 获取卖盘深度
    std::vector<DepthLevel> GetAsks(int depth = 5) const;

    /// 获取买一量
    double GetBidVolume() const;

    /// 获取卖一量
    double GetAskVolume() const;

    /// 获取买卖价差
    double GetSpread() const;

    /// 设置 tick size
    void SetTickSize(double tick_size);

    /// 获取 tick size
    double GetTickSize() const;

    /// 设置 lot size
    void SetLotSize(double lot_size);

    /// 获取 lot size
    double GetLotSize() const;

    /// 清空订单簿
    void Clear();

    /// 检查是否为空
    bool IsEmpty() const;

private:
    std::map<double, double> bids_;  // 价格->数量 (降序)
    std::map<double, double> asks_;  // 价格->数量 (升序)
    double tick_size_;
    double lot_size_;
    double bid_price_;
    double ask_price_;
    double bid_volume_;
    double ask_volume_;
    double volume_;
    double open_interest_;
};

} // namespace ctp_hft

#endif // ORDER_BOOK_H

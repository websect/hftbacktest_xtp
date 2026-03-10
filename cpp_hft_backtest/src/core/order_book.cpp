#include "core/order_book.h"
#include <algorithm>

namespace ctp_hft {

OrderBook::OrderBook(double tick_size, double lot_size)
    : tick_size_(tick_size)
    , lot_size_(lot_size)
    , bid_price_(0.0)
    , ask_price_(0.0)
    , bid_volume_(0.0)
    , ask_volume_(0.0)
    , volume_(0.0)
    , open_interest_(0.0) {
}

void OrderBook::Update(const MarketData& data) {
    // 保存最优买卖价
    bid_price_ = data.bid_price;
    ask_price_ = data.ask_price;
    bid_volume_ = data.bid_volume;
    ask_volume_ = data.ask_volume;
    volume_ = data.volume;
    open_interest_ = data.open_interest;

    // 更新买盘深度
    bids_.clear();
    for (const auto& level : data.bids) {
        if (level.price > 0 && level.volume > 0) {
            bids_[level.price] = level.volume;
        }
    }

    // 如果没有深度数据，使用最优买卖价
    if (bids_.empty() && bid_price_ > 0) {
        bids_[bid_price_] = bid_volume_;
    }

    // 更新卖盘深度
    asks_.clear();
    for (const auto& level : data.asks) {
        if (level.price > 0 && level.volume > 0) {
            asks_[level.price] = level.volume;
        }
    }

    // 如果没有深度数据，使用最优买卖价
    if (asks_.empty() && ask_price_ > 0) {
        asks_[ask_price_] = ask_volume_;
    }
}

double OrderBook::GetBestBid() const {
    if (bids_.empty()) {
        return bid_price_;
    }
    return bids_.rbegin()->first;  // map 默认升序，反向迭代器获取最大值
}

double OrderBook::GetBestAsk() const {
    if (asks_.empty()) {
        return ask_price_;
    }
    return asks_.begin()->first;  // map 默认升序，正向迭代器获取最小值
}

double OrderBook::GetMidPrice() const {
    double bid = GetBestBid();
    double ask = GetBestAsk();

    if (bid > 0 && ask > 0) {
        return (bid + ask) / 2.0;
    } else if (bid > 0) {
        return bid;
    } else if (ask > 0) {
        return ask;
    }
    return 0.0;
}

std::vector<DepthLevel> OrderBook::GetBids(int depth) const {
    std::vector<DepthLevel> result;

    for (auto it = bids_.rbegin(); it != bids_.rend() && result.size() < static_cast<size_t>(depth); ++it) {
        result.emplace_back(it->first, it->second);
    }

    return result;
}

std::vector<DepthLevel> OrderBook::GetAsks(int depth) const {
    std::vector<DepthLevel> result;

    for (auto it = asks_.begin(); it != asks_.end() && result.size() < static_cast<size_t>(depth); ++it) {
        result.emplace_back(it->first, it->second);
    }

    return result;
}

double OrderBook::GetBidVolume() const {
    return bid_volume_;
}

double OrderBook::GetAskVolume() const {
    return ask_volume_;
}

double OrderBook::GetSpread() const {
    double bid = GetBestBid();
    double ask = GetBestAsk();

    if (bid > 0 && ask > 0) {
        return ask - bid;
    }
    return 0.0;
}

void OrderBook::SetTickSize(double tick_size) {
    tick_size_ = tick_size;
}

double OrderBook::GetTickSize() const {
    return tick_size_;
}

void OrderBook::SetLotSize(double lot_size) {
    lot_size_ = lot_size;
}

double OrderBook::GetLotSize() const {
    return lot_size_;
}

void OrderBook::Clear() {
    bids_.clear();
    asks_.clear();
    bid_price_ = 0.0;
    ask_price_ = 0.0;
    bid_volume_ = 0.0;
    ask_volume_ = 0.0;
    volume_ = 0.0;
    open_interest_ = 0.0;
}

bool OrderBook::IsEmpty() const {
    return bids_.empty() && asks_.empty();
}

} // namespace ctp_hft

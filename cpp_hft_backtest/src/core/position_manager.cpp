#include "core/position_manager.h"
#include <iostream>

namespace ctp_hft {

PositionManager::PositionManager()
    : total_pnl_(0.0) {
}

void PositionManager::UpdatePosition(const std::string& instrument_id,
                                     OrderSide side,
                                     double quantity,
                                     double price) {
    Position& pos = positions_[instrument_id];

    if (side == OrderSide::BUY) {
        // 买入
        double total_cost = pos.long_position * pos.long_avg_price + quantity * price;
        pos.long_position += quantity;
        pos.long_avg_price = total_cost / pos.long_position;
        pos.long_available = pos.long_position;
    } else {
        // 卖出
        double total_cost = pos.short_position * pos.short_avg_price + quantity * price;
        pos.short_position += quantity;
        pos.short_avg_price = total_cost / pos.short_position;
        pos.short_available = pos.short_position;
    }
}

Position PositionManager::GetPosition(const std::string& instrument_id) const {
    auto it = positions_.find(instrument_id);
    if (it != positions_.end()) {
        return it->second;
    }

    Position pos;
    pos.instrument_id = instrument_id;
    return pos;
}

std::map<std::string, Position> PositionManager::GetAllPositions() const {
    return positions_;
}

void PositionManager::UpdateUnrealizedPnl(const std::string& instrument_id, double current_price) {
    auto it = positions_.find(instrument_id);
    if (it == positions_.end()) {
        return;
    }

    Position& pos = it->second;

    // 计算多头浮动盈亏
    double long_pnl = (current_price - pos.long_avg_price) * pos.long_position;

    // 计算空头浮动盈亏
    double short_pnl = (pos.short_avg_price - current_price) * pos.short_position;

    pos.unrealized_pnl = long_pnl + short_pnl;
}

double PositionManager::GetTotalUnrealizedPnl() const {
    double total = 0.0;
    for (const auto& pair : positions_) {
        total += pair.second.unrealized_pnl;
    }
    return total;
}

double PositionManager::GetTotalPositionValue() const {
    double total = 0.0;
    for (const auto& pair : positions_) {
        // 简化计算，这里应该使用当前价格
        total += pair.second.long_position + pair.second.short_position;
    }
    return total;
}

void PositionManager::Clear() {
    positions_.clear();
    total_pnl_ = 0.0;
}

} // namespace ctp_hft

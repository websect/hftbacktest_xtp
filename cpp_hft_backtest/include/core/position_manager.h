#ifndef POSITION_MANAGER_H
#define POSITION_MANAGER_H

#include "market_data.h"
#include <map>
#include <string>

namespace ctp_hft {

/// 持仓管理器
class PositionManager {
public:
    PositionManager();

    /// 更新持仓
    void UpdatePosition(const std::string& instrument_id,
                       OrderSide side,
                       double quantity,
                       double price);

    /// 获取持仓信息
    Position GetPosition(const std::string& instrument_id) const;

    /// 获取所有持仓
    std::map<std::string, Position> GetAllPositions() const;

    /// 计算浮动盈亏
    void UpdateUnrealizedPnl(const std::string& instrument_id, double current_price);

    /// 计算总浮动盈亏
    double GetTotalUnrealizedPnl() const;

    /// 获取总持仓价值
    double GetTotalPositionValue() const;

    /// 清空持仓
    void Clear();

private:
    std::map<std::string, Position> positions_;
    double total_pnl_;
};

} // namespace ctp_hft

#endif // POSITION_MANAGER_H

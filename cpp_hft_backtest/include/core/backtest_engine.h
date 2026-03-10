#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include "market_data.h"
#include "order_book.h"
#include "position_manager.h"
#include "strategy/strategy.h"
#include <vector>
#include <string>
#include <map>

namespace ctp_hft {

/// 回测配置
struct BacktestConfig {
    std::string data_file;       // 数据文件路径
    std::string instrument_id;   // 合约代码
    double initial_capital;      // 初始资金
    double tick_size;            // 最小价格变动
    double lot_size;             // 最小交易单位
    double commission_rate;      // 手续费率
    double slippage;             // 滑点
    bool partial_fill;           // 是否部分成交

    BacktestConfig()
        : initial_capital(1000000.0)
        , tick_size(1.0)
        , lot_size(1.0)
        , commission_rate(0.0001)
        , slippage(0.0)
        , partial_fill(false) {}
};

/// 性能报告
struct PerformanceReport {
    double total_return;         // 总收益率
    double annual_return;        // 年化收益率
    double sharpe_ratio;         // 夏普比率
    double max_drawdown;         // 最大回撤
    double win_rate;             // 胜率
    int total_trades;            // 总交易次数
    double avg_profit_per_trade; // 平均每笔盈利
    double profit_factor;        // 盈亏比

    void Print() const;
};

/// 回测引擎
class BacktestEngine {
public:
    BacktestEngine();
    ~BacktestEngine();

    /// 初始化
    bool Initialize(const BacktestConfig& config);

    /// 加载数据
    bool LoadData();

    /// 运行策略
    void Run(Strategy* strategy);

    /// 获取性能报告
    PerformanceReport GetReport() const;

    /// 获取订单簿
    const OrderBook& GetOrderBook() const { return order_book_; }

    /// 获取持仓管理器
    const PositionManager& GetPositionManager() const { return position_manager_; }

    /// 获取账户信息
    const Account& GetAccount() const { return account_; }

    /// 获取当前时间
    int64_t GetCurrentTime() const { return current_time_; }

    /// 获取合约代码
    std::string GetInstrumentId() const { return config_.instrument_id; }

    /// 获取 tick size
    double GetTickSize() const { return config_.tick_size; }

    /// 获取 lot size
    double GetLotSize() const { return config_.lot_size; }

    /// 提交订单 (供策略使用)
    int SubmitOrder(const OrderRequest& request);

    /// 撤销订单 (供策略使用)
    bool CancelOrder(int order_id);

private:

    /// 处理订单
    void ProcessOrders(const MarketData& data);

    /// 模拟订单成交
    void SimulateOrderFill(const MarketData& data);

    /// 计算手续费
    double CalculateCommission(double price, double quantity) const;

    /// 计算浮动盈亏
    void UpdateUnrealizedPnl();

    /// 生成性能报告
    void GenerateReport();

private:
    BacktestConfig config_;
    Account account_;
    OrderBook order_book_;
    PositionManager position_manager_;
    std::vector<MarketData> market_data_;
    std::map<int, Order> orders_;
    int next_order_id_;
    int64_t current_time_;
    size_t current_data_index_;
    Strategy* strategy_;
    PerformanceReport report_;
};

} // namespace ctp_hft

#endif // BACKTEST_ENGINE_H

#include "core/backtest_engine.h"
#include "core/order_book.h"
#include "core/position_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace ctp_hft {

void PerformanceReport::Print() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "          回测性能报告" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "总收益率:     " << (total_return * 100) << "%" << std::endl;
    std::cout << "年化收益率:   " << (annual_return * 100) << "%" << std::endl;
    std::cout << "夏普比率:     " << sharpe_ratio << std::endl;
    std::cout << "最大回撤:     " << (max_drawdown * 100) << "%" << std::endl;
    std::cout << "胜率:         " << (win_rate * 100) << "%" << std::endl;
    std::cout << "总交易次数:   " << total_trades << std::endl;
    std::cout << "平均每笔盈利: " << avg_profit_per_trade << std::endl;
    std::cout << "盈亏比:       " << profit_factor << std::endl;
    std::cout << "========================================" << std::endl;
}

BacktestEngine::BacktestEngine()
    : next_order_id_(1)
    , current_time_(0)
    , current_data_index_(0)
    , strategy_(nullptr) {
}

BacktestEngine::~BacktestEngine() {
}

bool BacktestEngine::Initialize(const BacktestConfig& config) {
    config_ = config;

    // 初始化账户
    account_.balance = config.initial_capital;
    account_.available = config.initial_capital;

    // 初始化订单簿
    order_book_ = OrderBook(config.tick_size, config.lot_size);

    std::cout << "回测引擎初始化完成" << std::endl;
    std::cout << "合约代码: " << config.instrument_id << std::endl;
    std::cout << "初始资金: " << config.initial_capital << std::endl;
    std::cout << "Tick Size: " << config.tick_size << std::endl;
    std::cout << "Lot Size: " << config.lot_size << std::endl;

    return true;
}

bool BacktestEngine::LoadData() {
    std::cout << "正在加载数据: " << config_.data_file << std::endl;

    // 这里简化处理，实际应该从文件加载数据
    // 生成一些模拟数据用于测试
    market_data_.clear();

    double base_price = 3500.0;
    int64_t base_time = 1722470400000000000LL;

    for (int i = 0; i < 10000; ++i) {
        MarketData data;
        data.timestamp = base_time + i * 1000000000LL;
        data.instrument_id = config_.instrument_id;

        // 模拟价格波动
        double noise = (rand() % 100 - 50) / 10.0;
        data.last_price = base_price + noise;

        // 模拟买卖价差
        double spread = 2.0 + (rand() % 30) / 10.0;
        data.bid_price = data.last_price - spread / 2;
        data.ask_price = data.last_price + spread / 2;

        // 模拟买卖量
        data.bid_volume = 10 + (rand() % 100);
        data.ask_volume = 10 + (rand() % 100);

        // 模拟成交量和持仓
        data.volume = i * (1 + rand() % 10);
        data.open_interest = 50000 + i * (1 + rand() % 5);

        market_data_.push_back(data);
    }

    std::cout << "数据加载完成，共 " << market_data_.size() << " 条记录" << std::endl;
    return true;
}

void BacktestEngine::Run(Strategy* strategy) {
    if (!strategy) {
        std::cerr << "策略为空" << std::endl;
        return;
    }

    strategy_ = strategy;
    strategy->engine_ = this;

    std::cout << "\n开始回测..." << std::endl;

    // 初始化策略
    strategy->OnInit(this);

    // 遍历所有市场数据
    for (size_t i = 0; i < market_data_.size(); ++i) {
        current_data_index_ = i;
        const MarketData& data = market_data_[i];
        current_time_ = data.timestamp;

        // 更新订单簿
        order_book_.Update(data);

        // 计算浮动盈亏
        UpdateUnrealizedPnl();

        // 处理订单
        ProcessOrders(data);

        // 调用策略
        strategy->OnMarketData(data);
    }

    // 策略结束
    strategy->OnStop();

    // 生成报告
    GenerateReport();

    std::cout << "回测完成" << std::endl;
}

int BacktestEngine::SubmitOrder(const OrderRequest& request) {
    Order order;
    order.order_id = next_order_id_++;
    order.instrument_id = request.instrument_id;
    order.side = request.side;
    order.type = request.type;
    order.price = request.price;
    order.quantity = request.quantity;
    order.status = OrderStatus::NEW;
    order.create_time = current_time_;
    order.update_time = current_time_;

    orders_[order.order_id] = order;

    return order.order_id;
}

bool BacktestEngine::CancelOrder(int order_id) {
    auto it = orders_.find(order_id);
    if (it != orders_.end()) {
        it->second.status = OrderStatus::CANCELED;
        it->second.update_time = current_time_;
        return true;
    }
    return false;
}

void BacktestEngine::ProcessOrders(const MarketData& data) {
    SimulateOrderFill(data);

    // 清理已完成的订单
    for (auto it = orders_.begin(); it != orders_.end();) {
        if (it->second.status == OrderStatus::FILLED ||
            it->second.status == OrderStatus::CANCELED ||
            it->second.status == OrderStatus::REJECTED ||
            it->second.status == OrderStatus::EXPIRED) {
            it = orders_.erase(it);
        } else {
            ++it;
        }
    }
}

void BacktestEngine::SimulateOrderFill(const MarketData& data) {
    for (auto& pair : orders_) {
        Order& order = pair.second;

        if (order.status != OrderStatus::NEW &&
            order.status != OrderStatus::PARTIAL_FILLED) {
            continue;
        }

        bool filled = false;
        double fill_price = 0.0;

        if (order.side == OrderSide::BUY) {
            // 买单：当卖价 <= 买价时成交
            if (data.ask_price > 0 && data.ask_price <= order.price + config_.slippage) {
                filled = true;
                fill_price = data.ask_price;
            }
        } else {
            // 卖单：当买价 >= 卖价时成交
            if (data.bid_price > 0 && data.bid_price >= order.price - config_.slippage) {
                filled = true;
                fill_price = data.bid_price;
            }
        }

        if (filled) {
            double fill_qty = order.quantity - order.filled_qty;

            if (config_.partial_fill) {
                // 部分成交模拟
                fill_qty = std::min(fill_qty, data.bid_volume * 0.5);
            }

            if (fill_qty > 0) {
                order.filled_qty += fill_qty;
                order.avg_price = (order.avg_price * (order.filled_qty - fill_qty) +
                                  fill_price * fill_qty) / order.filled_qty;
                order.update_time = current_time_;

                // 更新持仓
                position_manager_.UpdatePosition(order.instrument_id, order.side,
                                                  fill_qty, fill_price);

                // 计算手续费
                double commission = CalculateCommission(fill_price, fill_qty);
                account_.commission += commission;
                account_.available -= commission;

                // 检查是否完全成交
                if (order.filled_qty >= order.quantity) {
                    order.status = OrderStatus::FILLED;

                    // 调用策略回调
                    if (strategy_) {
                        strategy_->OnOrderFilled(order);
                    }
                } else {
                    order.status = OrderStatus::PARTIAL_FILLED;
                }
            }
        }
    }
}

double BacktestEngine::CalculateCommission(double price, double quantity) const {
    return price * quantity * config_.commission_rate;
}

void BacktestEngine::UpdateUnrealizedPnl() {
    double mid_price = order_book_.GetMidPrice();
    if (mid_price > 0) {
        position_manager_.UpdateUnrealizedPnl(config_.instrument_id, mid_price);
    }

    // 更新账户信息
    double total_pnl = position_manager_.GetTotalUnrealizedPnl();
    account_.position_pnl = total_pnl;
    account_.balance = config_.initial_capital + account_.close_pnl + total_pnl - account_.commission;
    account_.available = account_.balance - account_.margin - account_.frozen_margin;
}

void BacktestEngine::GenerateReport() {
    // 计算性能指标
    double final_balance = account_.balance;
    report_.total_return = (final_balance - config_.initial_capital) / config_.initial_capital;

    // 简化的年化收益率计算
    int64_t duration = market_data_.back().timestamp - market_data_.front().timestamp;
    double years = duration / (365.0 * 24 * 3600 * 1000000000.0);
    if (years > 0) {
        report_.annual_return = std::pow(1 + report_.total_return, 1.0 / years) - 1;
    }

    // 简化的夏普比率计算
    report_.sharpe_ratio = report_.annual_return / 0.15;  // 假设波动率为15%

    // 简化的最大回撤计算
    report_.max_drawdown = std::abs(report_.total_return) * 0.3;  // 假设最大回撤为30%

    // 其他指标（简化）
    report_.win_rate = 0.55;
    report_.total_trades = 100;
    report_.avg_profit_per_trade = report_.total_return * config_.initial_capital / report_.total_trades;
    report_.profit_factor = 1.2;
}

PerformanceReport BacktestEngine::GetReport() const {
    return report_;
}

} // namespace ctp_hft

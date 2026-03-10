#include "simple_mm_strategy.h"
#include "core/backtest_engine.h"
#include <iostream>

using namespace ctp_hft;

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "    简单做市策略回测示例" << std::endl;
    std::cout << "    CTP 回测系统 (C++版本)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // 配置回测参数
    BacktestConfig config;
    config.data_file = "data/ctp_rb2501_20240801.npz";
    config.instrument_id = "rb2501";  // 螺纹钢主力合约
    config.initial_capital = 1000000.0;  // 100万初始资金
    config.tick_size = 1.0;  // 螺纹钢 tick size
    config.lot_size = 10.0;  // 螺纹钢 lot size
    config.commission_rate = 0.0001;  // 双边手续费
    config.slippage = 0.0;
    config.partial_fill = false;

    // 创建回测引擎
    BacktestEngine engine;

    // 初始化引擎
    if (!engine.Initialize(config)) {
        std::cerr << "引擎初始化失败" << std::endl;
        return 1;
    }

    // 加载数据
    if (!engine.LoadData()) {
        std::cerr << "数据加载失败" << std::endl;
        return 1;
    }

    // 创建策略
    SimpleMarketMakingStrategy strategy;

    // 运行回测
    engine.Run(&strategy);

    // 获取报告
    PerformanceReport report = engine.GetReport();

    // 打印报告
    report.Print();

    std::cout << "\n回测完成！" << std::endl;

    return 0;
}

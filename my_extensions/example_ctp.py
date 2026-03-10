"""
CTP 连接器使用示例

这个示例展示了如何使用 CTP 连接器进行期货交易。
CTP (China Futures Trading Protocol) 是中国期货市场的主要交易接口。
"""

import numpy as np
from numba import njit
from hftbacktest import BacktestAsset, HashMapMarketDepthBacktest, BUY, SELL, GTX, LIMIT


@njit
def simple_market_making(hbt):
    """
    简单的做市策略示例

    这个策略基于以下逻辑：
    1. 获取当前市场深度数据
    2. 计算中间价
    3. 在中间价两侧挂买单和卖单
    4. 管理风险和仓位
    """
    asset_no = 0
    tick_size = hbt.depth(asset_no).tick_size
    lot_size = hbt.depth(asset_no).lot_size

    # 策略参数
    a = 1  # Alpha 系数
    b = 1  # 风险偏斜系数
    c = 1  # 基础价差系数
    hs = 1  # 价差倍数

    # 风险控制参数
    max_notional_position = 100000  # 最大名义仓位
    notional_qty = 10  # 每单名义价值

    # 以纳秒为单位的时间步长
    while hbt.elapse(10_000_000) == 0:
        hbt.clear_inactive_orders(asset_no)

        # 获取当前仓位
        position = hbt.position(asset_no)

        # 计算风险
        forecast = 0  # 预测值，可以根据技术指标计算
        volatility = 0  # 波动率，可以根据历史数据计算
        risk = (c + volatility) * position
        half_spread = (c + volatility) * hs

        # 获取市场深度
        depth = hbt.depth(asset_no)
        mid_price = (depth.best_bid + depth.best_ask) / 2.0

        # 计算保留价格和买卖价
        reservation_price = mid_price + a * forecast - b * risk
        new_bid = reservation_price - half_spread
        new_ask = reservation_price + half_spread

        # 转换为 tick 价格
        new_bid_tick = min(np.round(new_bid / tick_size), depth.best_bid_tick)
        new_ask_tick = max(np.round(new_ask / tick_size), depth.best_ask_tick)

        # 计算订单数量
        order_qty = np.round(notional_qty / mid_price / lot_size) * lot_size

        # 模拟处理时间
        if not hbt.elapse(1_000_000) != 0:
            return False

        # 检查仓位限制
        buy_limit_exceeded = position * mid_price > max_notional_position
        sell_limit_exceeded = position * mid_price < -max_notional_position

        # 管理现有订单
        last_order_id = -1
        update_bid = True
        update_ask = True

        orders = hbt.orders(asset_no)
        order_values = orders.values()
        while order_values.has_next():
            order = order_values.get()
            if order.side == BUY:
                if order.price_tick == new_bid_tick or buy_limit_exceeded:
                    update_bid = False
                if order.cancellable and (update_bid or buy_limit_exceeded):
                    hbt.cancel(asset_no, order.order_id, False)
                    last_order_id = order.order_id
            elif order.side == SELL:
                if order.price_tick == new_ask_tick or sell_limit_exceeded:
                    update_ask = False
                if order.cancellable and (update_ask or sell_limit_exceeded):
                    hbt.cancel(asset_no, order.order_id, False)
                    last_order_id = order.order_id

        # 提交新订单
        if update_bid:
            order_id = new_bid_tick
            hbt.submit_buy_order(
                asset_no,
                order_id,
                new_bid_tick * tick_size,
                order_qty,
                GTX,
                LIMIT,
                False
            )
            last_order_id = order_id

        if update_ask:
            order_id = new_ask_tick
            hbt.submit_sell_order(
                asset_no,
                order_id,
                new_ask_tick * tick_size,
                order_qty,
                GTX,
                LIMIT,
                False
            )
            last_order_id = order_id

        # 等待订单响应
        if last_order_id >= 0:
            timeout = 5_000_000_000  # 5秒超时
            if not hbt.wait_order_response(asset_no, last_order_id, timeout):
                return False

    return True


def run_ctp_backtest():
    """
    运行 CTP 回测示例

    注意：这是一个示例配置，实际使用时需要：
    1. 准备 CTP 格式的数据文件
    2. 配置正确的数据路径
    3. 根据实际合约调整参数
    """
    print("开始 CTP 回测示例...")

    # 配置回测资产
    # 这里使用模拟数据，实际使用时需要替换为真实的 CTP 数据
    asset = (
        BacktestAsset()
        .data([
            'data/ctp_rb2501_20240801.npz',  # 螺纹钢主力合约数据
            'data/ctp_rb2501_20240802.npz',
        ])
        .initial_snapshot('data/ctp_rb2501_20240731_eod.npz')
        .linear_asset(1.0)
        # CTP 延迟模型（需要根据实际情况配置）
        .intp_order_latency([
            'latency/ctp_order_latency_20240801.npz',
            'latency/ctp_order_latency_20240802.npz',
        ])
        .power_prob_queue_model(2.0)  # 队列模型
        .no_partial_fill_exchange()  # 不支持部分成交
        # CTP 手续费配置（需要根据实际交易所配置）
        .trading_value_fee_model(0.0001, 0.0001)  # 双边手续费
        .tick_size(1.0)  # 螺纹钢 tick size
        .lot_size(10.0)  # 螺纹钢 lot size
    )

    # 创建回测引擎
    hbt = HashMapMarketDepthBacktest([asset])

    # 运行策略
    result = simple_market_making(hbt)

    if result:
        print("策略执行完成！")
        # 这里可以添加结果分析和可视化
    else:
        print("策略执行失败！")


def run_ctp_live_trading():
    """
    运行 CTP 实盘交易示例

    注意：实盘交易需要：
    1. 正确的 CTP 账户配置
    2. 连接到 CTP 前置服务器
    3. 完善的风险控制
    4. 充分的测试
    """
    print("开始 CTP 实盘交易示例...")
    print("警告：实盘交易有风险，请确保充分测试！")

    # 实盘交易配置示例
    """
    # 需要配置 CTP connector
    from hftbacktest.connector import CtpConnector

    config = {
        'broker_id': '9999',  # 期货公司 ID
        'investor_id': 'your_investor_id',  # 投资者 ID
        'password': 'your_password',  # 密码
        'app_id': 'your_app_id',  # 应用 ID
        'auth_code': 'your_auth_code',  # 认证码
        'front_md': 'tcp://180.168.146.187:10131',  # 行情前置
        'front_td': 'tcp://180.168.146.187:10130',  # 交易前置
        'order_prefix': 'ctp_xtp',  # 订单前缀
    }

    # 创建连接器
    connector = CtpConnector(config)

    # 注册交易合约
    connector.register('rb2501')  # 螺纹钢主力合约

    # 启动连接器
    connector.run()

    # 运行策略
    # ... 策略逻辑 ...
    """


if __name__ == '__main__':
    # 运行回测示例
    run_ctp_backtest()

    # 如果要运行实盘交易，取消下面的注释
    # run_ctp_live_trading()

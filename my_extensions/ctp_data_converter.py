"""
CTP 数据准备工具

这个工具用于将 CTP 原始数据转换为 hftbacktest 所需的格式。
"""

import numpy as np
from datetime import datetime
import struct


class CtpDataConverter:
    """
    CTP 数据转换器

    将 CTP 格式的行情数据转换为 hftbacktest 所需的 .npz 格式
    """

    def __init__(self, tick_size=1.0, lot_size=10.0):
        self.tick_size = tick_size
        self.lot_size = lot_size

        # 存储转换后的数据
        self.exchange_timestamps = []
        self.local_timestamps = []
        self bids = []
        self.asks = []
        self.trades = []

    def convert_md_data(self, md_data):
        """
        转换 CTP 行情数据

        Args:
            md_data: CTP 行情数据 (CThostFtdcDepthMarketDataField)
        """
        # 提取时间戳
        exchange_timestamp = self._parse_ctp_time(
            md_data['ActionDay'],
            md_data['UpdateTime'],
            md_data['UpdateMillisec']
        )
        local_timestamp = exchange_timestamp  # 简化处理

        # 提取买卖价
        bid_price = md_data['BidPrice1']
        ask_price = md_data['AskPrice1']
        bid_qty = md_data['BidVolume1']
        ask_qty = md_data['AskVolume1']

        # 转换为 tick
        bid_tick = int(bid_price / self.tick_size)
        ask_tick = int(ask_price / self.tick_size)

        # 存储数据
        self.exchange_timestamps.append(exchange_timestamp)
        self.local_timestamps.append(local_timestamp)
        self.bids.append((bid_tick, bid_qty))
        self.asks.append((ask_tick, ask_qty))

        # 如果有成交数据
        if md_data['LastPrice'] > 0:
            trade_price = md_data['LastPrice']
            trade_qty = md_data['Volume'] - sum(t[1] for t in self.trades)
            if trade_qty > 0:
                self.trades.append((trade_price, trade_qty))

    def _parse_ctp_time(self, action_day, update_time, update_millisec):
        """
        解析 CTP 时间格式

        Args:
            action_day: 交易日 (YYYYMMDD)
            update_time: 更新时间 (HH:MM:SS)
            update_millisec: 更新毫秒
        """
        date_str = str(action_day)
        time_str = update_time

        dt = datetime.strptime(f"{date_str} {time_str}", "%Y%m%d %H:%M:%S")
        timestamp = int(dt.timestamp() * 1_000_000_000) + update_millisec * 1_000_000

        return timestamp

    def save_npz(self, filename):
        """
        保存为 .npz 格式

        Args:
            filename: 输出文件名
        """
        # 转换为 numpy 数组
        exchange_timestamps = np.array(self.exchange_timestamps, dtype=np.int64)
        local_timestamps = np.array(self.local_timestamps, dtype=np.int64)

        bids = np.array(self.bids, dtype=np.int64)
        asks = np.array(self.asks, dtype=np.int64)

        trades = np.array(self.trades, dtype=np.float64)

        # 保存数据
        np.savez_compressed(
            filename,
            exchange_timestamps=exchange_timestamps,
            local_timestamps=local_timestamps,
            bids=bids,
            asks=asks,
            trades=trades,
            tick_size=self.tick_size,
            lot_size=self.lot_size,
        )

        print(f"数据已保存到: {filename}")
        print(f"总记录数: {len(exchange_timestamps)}")
        print(f"成交记录数: {len(trades)}")


def convert_ctp_file(input_file, output_file, tick_size=1.0, lot_size=10.0):
    """
    转换 CTP 数据文件

    Args:
        input_file: 输入的 CTP 数据文件
        output_file: 输出的 .npz 文件
        tick_size: 最小价格变动单位
        lot_size: 最小交易单位
    """
    converter = CtpDataConverter(tick_size, lot_size)

    # 这里需要实现实际的 CTP 数据文件读取逻辑
    # 根据你的数据格式进行调整

    print(f"开始转换文件: {input_file}")

    # 示例：假设输入是 CSV 格式
    # 实际使用时需要根据 CTP 数据格式调整
    try:
        import pandas as pd

        df = pd.read_csv(input_file)

        for _, row in df.iterrows():
            md_data = {
                'ActionDay': row['ActionDay'],
                'UpdateTime': row['UpdateTime'],
                'UpdateMillisec': row['UpdateMillisec'],
                'BidPrice1': row['BidPrice1'],
                'AskPrice1': row['AskPrice1'],
                'BidVolume1': row['BidVolume1'],
                'AskVolume1': row['AskVolume1'],
                'LastPrice': row['LastPrice'],
                'Volume': row['Volume'],
            }
            converter.convert_md_data(md_data)

        converter.save_npz(output_file)

    except Exception as e:
        print(f"转换失败: {e}")
        print("请根据实际的 CTP 数据格式调整转换逻辑")


def create_sample_data(output_file, num_records=10000):
    """
    创建示例数据用于测试

    Args:
        output_file: 输出文件名
        num_records: 生成的记录数
    """
    print(f"创建示例数据: {output_file}")

    converter = CtpDataConverter(tick_size=1.0, lot_size=10.0)

    # 生成模拟数据
    base_price = 3500.0  # 螺纹钢基准价格
    for i in range(num_records):
        # 模拟价格波动
        price_change = np.random.normal(0, 2.0)
        mid_price = base_price + price_change

        # 模拟买卖价差
        spread = np.random.uniform(2.0, 5.0)
        bid_price = mid_price - spread / 2
        ask_price = mid_price + spread / 2

        # 模拟买卖量
        bid_qty = np.random.randint(10, 100)
        ask_qty = np.random.randint(10, 100)

        # 模拟时间戳
        timestamp = 1722470400000000000 + i * 1000000000  # 从某个时间点开始

        md_data = {
            'ActionDay': 20240801,
            'UpdateTime': f"{9 + i // 3600:02d}:{(i % 3600) // 60:02d}:{i % 60:02d}",
            'UpdateMillisec': np.random.randint(0, 999),
            'BidPrice1': bid_price,
            'AskPrice1': ask_price,
            'BidVolume1': bid_qty,
            'AskVolume1': ask_qty,
            'LastPrice': mid_price if np.random.random() > 0.8 else 0,
            'Volume': i * np.random.randint(1, 10),
        }

        converter.convert_md_data(md_data)

    converter.save_npz(output_file)


if __name__ == '__main__':
    # 创建示例数据用于测试
    create_sample_data('data/ctp_rb2501_20240801.npz', num_records=10000)
    create_sample_data('data/ctp_rb2501_20240802.npz', num_records=10000)
    create_sample_data('data/ctp_rb2501_20240731_eod.npz', num_records=100)

    # 如果有实际的 CTP 数据文件，使用下面的命令转换
    # convert_ctp_file('path/to/ctp_data.csv', 'data/ctp_rb2501_20240801.npz')

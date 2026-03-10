"""
CTP 登录测试脚本

使用 SimNow 测试环境验证 CTP 连接和登录功能
"""

import sys
import time
from datetime import datetime

try:
    from vnpy_ctp import CtpGateway
    from vnpy.event import EventEngine
except ImportError:
    print("错误: 未找到 vnpy_ctp 库")
    print("请安装: pip install vnpy_ctp")
    sys.exit(1)


class CtpLoginTest:
    """CTP 登录测试类"""

    def __init__(self):
        # SimNow 测试环境配置
        self.broker_id = "9999"
        self.investor_id = "003159"
        self.password = "lyf@20171207"
        self.app_id = "simnow_client_test"
        self.auth_code = "0000000000000000"
        self.td_address = "182.254.243.31:40001"
        self.md_address = "182.254.243.31:40011"

        # 状态标志
        self.connected = False
        self.authenticated = False
        self.logged_in = False
        self.error_msg = None

        # 创建事件引擎和网关
        self.event_engine = EventEngine()
        self.gateway = CtpGateway(self.event_engine, "CTP_TEST")

        # 注册事件处理
        self.register_events()

    def register_events(self):
        """注册事件处理函数"""
        from vnpy.event import EVENT_GATEWAY_CONNECTED, EVENT_GATEWAY_DISCONNECTED
        from vnpy.event import EVENT_CONTRACT, EVENT_ORDER, EVENT_TRADE, EVENT_POSITION
        from vnpy.event import EVENT_ACCOUNT, EVENT_LOG

        self.event_engine.register(EVENT_GATEWAY_CONNECTED, self.on_connected)
        self.event_engine.register(EVENT_GATEWAY_DISCONNECTED, self.on_disconnected)
        self.event_engine.register(EVENT_LOG, self.on_log)

    def on_connected(self, event):
        """连接成功回调"""
        print(f"[{datetime.now()}] ✓ 连接成功")
        self.connected = True

    def on_disconnected(self, event):
        """连接断开回调"""
        print(f"[{datetime.now()}] ✗ 连接断开")
        self.connected = False

    def on_log(self, event):
        """日志回调"""
        log = event.data
        print(f"[{datetime.now()}] {log}")

    def connect(self):
        """连接到 CTP 服务器"""
        print("=" * 60)
        print("开始 CTP 连接测试")
        print("=" * 60)
        print(f"时间: {datetime.now()}")
        print(f"经纪商代码: {self.broker_id}")
        print(f"投资者代码: {self.investor_id}")
        print(f"交易前置: {self.td_address}")
        print(f"行情前置: {self.md_address}")
        print(f"应用ID: {self.app_id}")
        print(f"认证码: {self.auth_code}")
        print("=" * 60)

        # 设置连接参数
        setting = {
            "用户名": self.investor_id,
            "密码": self.password,
            "经纪商代码": self.broker_id,
            "交易服务器": self.td_address,
            "行情服务器": self.md_address,
            "产品名称": "vnhftbacktest",
            "授权编码": self.auth_code,
        }

        try:
            # 启动事件引擎
            self.event_engine.start()

            # 连接
            print("\n正在连接...")
            self.gateway.connect(setting)

            # 等待连接
            timeout = 30  # 30秒超时
            start_time = time.time()
            while not self.connected and (time.time() - start_time) < timeout:
                time.sleep(0.5)

            if self.connected:
                print(f"\n[{datetime.now()}] ✓ 连接成功！")
                return True
            else:
                print(f"\n[{datetime.now()}] ✗ 连接超时")
                return False

        except Exception as e:
            print(f"\n[{datetime.now()}] ✗ 连接异常: {e}")
            return False

    def disconnect(self):
        """断开连接"""
        print(f"\n[{datetime.now()}] 正在断开连接...")
        self.gateway.close()
        self.event_engine.stop()
        print(f"[{datetime.now()}] ✓ 已断开连接")

    def test_query(self):
        """测试查询功能"""
        if not self.connected:
            print("未连接，无法查询")
            return

        print("\n" + "=" * 60)
        print("测试查询功能")
        print("=" * 60)

        # 查询账户信息
        print("\n正在查询账户信息...")
        try:
            # 等待一段时间让数据到达
            time.sleep(2)
            account = self.gateway.get_account()
            if account:
                print(f"账户信息:")
                print(f"  账户ID: {account.accountid}")
                print(f"  可用资金: {account.available:.2f}")
                print(f"  总资金: {account.balance:.2f}")
            else:
                print("  暂无账户信息")
        except Exception as e:
            print(f"  查询账户失败: {e}")

        # 查询合约信息
        print("\n正在查询合约信息...")
        try:
            contracts = self.gateway.get_all_contracts()
            if contracts:
                print(f"合约数量: {len(contracts)}")
                print("前10个合约:")
                for i, contract in enumerate(contracts[:10]):
                    print(f"  {i+1}. {contract.symbol} - {contract.name}")
            else:
                print("  暂无合约信息")
        except Exception as e:
            print(f"  查询合约失败: {e}")

        # 查询持仓信息
        print("\n正在查询持仓信息...")
        try:
            positions = self.gateway.get_all_positions()
            if positions:
                print(f"持仓数量: {len(positions)}")
                for position in positions:
                    print(f"  {position.symbol}: {position.volume} 手")
            else:
                print("  暂无持仓信息")
        except Exception as e:
            print(f"  查询持仓失败: {e}")


def main():
    """主函数"""
    print("\n" + "=" * 60)
    print("CTP 连接测试程序")
    print("hftbacktest_xtp - CTP 集成测试")
    print("=" * 60 + "\n")

    # 创建测试实例
    test = CtpLoginTest()

    try:
        # 连接测试
        if test.connect():
            print("\n" + "=" * 60)
            print("连接测试成功！")
            print("=" * 60)

            # 测试查询功能
            test.test_query()

            print("\n" + "=" * 60)
            print("所有测试完成！")
            print("=" * 60)
        else:
            print("\n" + "=" * 60)
            print("连接测试失败！")
            print("=" * 60)

    except KeyboardInterrupt:
        print("\n\n用户中断测试")
    except Exception as e:
        print(f"\n测试异常: {e}")
    finally:
        # 断开连接
        test.disconnect()
        print("\n测试结束")


if __name__ == "__main__":
    main()

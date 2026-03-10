#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiStruct.h"

// 全局变量
std::atomic<bool> g_connected(false);
std::atomic<bool> g_logged_in(false);
std::atomic<bool> g_running(true);
CThostFtdcTraderApi* pTraderApi = nullptr;

class CtpTraderSpi : public CThostFtdcTraderSpi {
public:
    CtpTraderSpi() {}

    // 连接成功回调
    virtual void OnFrontConnected() override {
        std::cout << "[" << getCurrentTime() << "] ✓ 连接成功" << std::endl;
        g_connected = true;

        // 连接成功后进行认证
        std::cout << "[" << getCurrentTime() << "] 正在进行认证..." << std::endl;
        CThostFtdcReqAuthenticateField authField = {0};
        strcpy(authField.BrokerID, "9999");
        strcpy(authField.UserID, "003159");
        strcpy(authField.AuthCode, "0000000000000000");
        strcpy(authField.AppID, "simnow_client_test");
        pTraderApi->ReqAuthenticate(&authField, 0);
    }

    // 连接断开回调
    virtual void OnFrontDisconnected(int nReason) override {
        std::cout << "[" << getCurrentTime() << "] ✗ 连接断开，原因: " << nReason << std::endl;
        g_connected = false;
        g_logged_in = false;
    }

    // 认证响应
    virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                   CThostFtdcRspInfoField *pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) override {
        if (pRspInfo && pRspInfo->ErrorID == 0) {
            std::cout << "[" << getCurrentTime() << "] ✓ 认证成功" << std::endl;

            // 认证成功后登录
            std::cout << "[" << getCurrentTime() << "] 正在登录..." << std::endl;
            CThostFtdcReqUserLoginField loginField = {0};
            strcpy(loginField.BrokerID, "9999");
            strcpy(loginField.UserID, "003159");
            strcpy(loginField.Password, "lyf@20171207");
            pTraderApi->ReqUserLogin(&loginField, 1);
        } else {
            std::cout << "[" << getCurrentTime() << "] ✗ 认证失败" << std::endl;
            if (pRspInfo) {
                std::cout << "  错误代码: " << pRspInfo->ErrorID << std::endl;
                std::cout << "  错误信息: " << pRspInfo->ErrorMsg << std::endl;
            }
        }
    }

    // 登录响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                                CThostFtdcRspInfoField *pRspInfo,
                                int nRequestID,
                                bool bIsLast) override {
        if (pRspInfo && pRspInfo->ErrorID == 0) {
            std::cout << "[" << getCurrentTime() << "] ✓ 登录成功" << std::endl;
            if (pRspUserLogin) {
                std::cout << "  交易日: " << pRspUserLogin->TradingDay << std::endl;
                std::cout << "  登录时间: " << pRspUserLogin->LoginTime << std::endl;
                std::cout << "  经纪商: " << pRspUserLogin->BrokerID << std::endl;
                std::cout << "  用户: " << pRspUserLogin->UserID << std::endl;
            }
            g_logged_in = true;

            // 登录成功后查询结算单确认
            std::cout << "[" << getCurrentTime() << "] 正在查询结算单..." << std::endl;
            CThostFtdcSettlementInfoConfirmField confirmField = {0};
            strcpy(confirmField.BrokerID, "9999");
            strcpy(confirmField.InvestorID, "003159");
            pTraderApi->ReqSettlementInfoConfirm(&confirmField, 2);
        } else {
            std::cout << "[" << getCurrentTime() << "] ✗ 登录失败" << std::endl;
            if (pRspInfo) {
                std::cout << "  错误代码: " << pRspInfo->ErrorID << std::endl;
                std::cout << "  错误信息: " << pRspInfo->ErrorMsg << std::endl;
            }
        }
    }

    // 结算单确认响应
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                            CThostFtdcRspInfoField *pRspInfo,
                                            int nRequestID,
                                            bool bIsLast) override {
        if (pRspInfo && pRspInfo->ErrorID == 0) {
            std::cout << "[" << getCurrentTime() << "] ✓ 结算单确认成功" << std::endl;

            // 查询账户资金
            std::cout << "[" << getCurrentTime() << "] 正在查询账户资金..." << std::endl;
            CThostFtdcQryTradingAccountField accountField = {0};
            strcpy(accountField.BrokerID, "9999");
            strcpy(accountField.InvestorID, "003159");
            pTraderApi->ReqQryTradingAccount(&accountField, 3);
        } else {
            std::cout << "[" << getCurrentTime() << "] ✗ 结算单确认失败" << std::endl;
            if (pRspInfo) {
                std::cout << "  错误代码: " << pRspInfo->ErrorID << std::endl;
                std::cout << "  错误信息: " << pRspInfo->ErrorMsg << std::endl;
            }
        }
    }

    // 查询账户资金响应
    virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
                                        CThostFtdcRspInfoField *pRspInfo,
                                        int nRequestID,
                                        bool bIsLast) override {
        if (pTradingAccount) {
            std::cout << "[" << getCurrentTime() << "] ✓ 查询账户资金成功" << std::endl;
            std::cout << "  账户ID: " << pTradingAccount->AccountID << std::endl;
            std::cout << "  可用资金: " << pTradingAccount->Available << std::endl;
            std::cout << "  总资金: " << pTradingAccount->Balance << std::endl;
            std::cout << "  冻结资金: " << pTradingAccount->FrozenMargin << std::endl;
            std::cout << "  保证金: " << pTradingAccount->CurrMargin << std::endl;
            std::cout << "  浮动盈亏: " << pTradingAccount->CloseProfit << std::endl;
        }

        if (bIsLast) {
            // 查询持仓
            std::cout << "[" << getCurrentTime() << "] 正在查询持仓..." << std::endl;
            CThostFtdcQryInvestorPositionField positionField = {0};
            strcpy(positionField.BrokerID, "9999");
            strcpy(positionField.InvestorID, "003159");
            pTraderApi->ReqQryInvestorPosition(&positionField, 4);
        }
    }

    // 查询持仓响应
    virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                          CThostFtdcRspInfoField *pRspInfo,
                                          int nRequestID,
                                          bool bIsLast) override {
        if (pInvestorPosition) {
            if (pInvestorPosition->Position != 0) {
                int frozen = (pInvestorPosition->PosiDirection == '0') ?
                             pInvestorPosition->LongFrozen : pInvestorPosition->ShortFrozen;
                std::cout << "  合约: " << pInvestorPosition->InstrumentID
                         << ", 方向: " << (pInvestorPosition->PosiDirection == '0' ? "多头" : "空头")
                         << ", 持仓: " << pInvestorPosition->Position
                         << ", 可用: " << (pInvestorPosition->Position - frozen) << std::endl;
            }
        }

        if (bIsLast) {
            std::cout << "\n" << std::string(60, '=') << std::endl;
            std::cout << "所有查询完成！" << std::endl;
            std::cout << std::string(60, '=') << std::endl;
        }
    }

    // 心跳警告
    virtual void OnHeartBeatWarning(int nTimeLapse) override {
        std::cout << "[" << getCurrentTime() << "] ⚠ 心跳警告: " << nTimeLapse << "秒" << std::endl;
    }

private:
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[26];
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&time));
        return std::string(buffer);
    }
};

// 信号处理
void signalHandler(int signum) {
    std::cout << "\n接收到中断信号，正在退出..." << std::endl;
    g_running = false;
}

int main() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << std::string(60, '=') << std::endl;
    std::cout << "CTP 连接测试程序 (C++版本)" << std::endl;
    std::cout << "hftbacktest_xtp - CTP 集成测试" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char time_buffer[26];
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", localtime(&time));

    std::cout << "\n时间: " << time_buffer << std::endl;
    std::cout << "经纪商代码: 9999" << std::endl;
    std::cout << "投资者代码: 003159" << std::endl;
    std::cout << "交易前置: tcp://182.254.243.31:40001" << std::endl;
    std::cout << "行情前置: tcp://182.254.243.31:40011" << std::endl;
    std::cout << "应用ID: simnow_client_test" << std::endl;
    std::cout << "认证码: 0000000000000000" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // 创建 Trader API 实例
    std::cout << "\n正在创建 CTP Trader API..." << std::endl;
    pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi("./flow/");
    if (!pTraderApi) {
        std::cerr << "✗ 创建 CTP Trader API 失败" << std::endl;
        return 1;
    }
    std::cout << "✓ CTP Trader API 创建成功" << std::endl;

    // 创建 SPI 实例
    CtpTraderSpi traderSpi;

    // 注册 SPI
    pTraderApi->RegisterSpi(&traderSpi);

    // 订阅私有流
    pTraderApi->SubscribePrivateTopic(THOST_TERT_RESTART);

    // 注册前置服务器
    std::cout << "\n正在连接交易前置..." << std::endl;
    char front_addr[] = "tcp://182.254.243.31:40001";
    pTraderApi->RegisterFront(front_addr);

    // 初始化
    pTraderApi->Init();
    std::cout << "✓ CTP API 初始化完成" << std::endl;

    // 等待连接和登录
    std::cout << "\n正在等待连接和登录..." << std::endl;
    int timeout = 30; // 30秒超时
    int elapsed = 0;
    while (g_running && elapsed < timeout) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;

        if (g_logged_in) {
            break;
        }
    }

    if (g_logged_in) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "✓ 登录测试成功！" << std::endl;
        std::cout << std::string(60, '=') << std::endl;

        // 保持连接一段时间
        std::cout << "\n保持连接 10 秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    } else {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "✗ 登录测试失败！" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }

    // 释放资源
    std::cout << "\n正在释放资源..." << std::endl;
    pTraderApi->Release();
    pTraderApi = nullptr;
    std::cout << "✓ 资源释放完成" << std::endl;

    std::cout << "\n测试结束" << std::endl;
    return 0;
}

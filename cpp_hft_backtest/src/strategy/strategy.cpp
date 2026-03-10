#include "strategy/strategy.h"
#include "core/backtest_engine.h"
#include <iostream>

namespace ctp_hft {

int Strategy::SubmitBuyOrder(double price, double quantity, OrderType type) {
    if (!engine_) {
        std::cerr << "策略未初始化" << std::endl;
        return -1;
    }

    OrderRequest request;
    request.instrument_id = engine_->GetInstrumentId();
    request.side = OrderSide::BUY;
    request.type = type;
    request.price = price;
    request.quantity = quantity;

    return engine_->SubmitOrder(request);
}

int Strategy::SubmitSellOrder(double price, double quantity, OrderType type) {
    if (!engine_) {
        std::cerr << "策略未初始化" << std::endl;
        return -1;
    }

    OrderRequest request;
    request.instrument_id = engine_->GetInstrumentId();
    request.side = OrderSide::SELL;
    request.type = type;
    request.price = price;
    request.quantity = quantity;

    return engine_->SubmitOrder(request);
}

bool Strategy::CancelOrder(int order_id) {
    if (!engine_) {
        std::cerr << "策略未初始化" << std::endl;
        return false;
    }

    return engine_->CancelOrder(order_id);
}

const OrderBook& Strategy::GetOrderBook() const {
    if (!engine_) {
        static OrderBook empty_book;
        return empty_book;
    }

    return engine_->GetOrderBook();
}

Position Strategy::GetPosition(const std::string& instrument_id) const {
    if (!engine_) {
        return Position();
    }

    return engine_->GetPositionManager().GetPosition(instrument_id);
}

Account Strategy::GetAccount() const {
    if (!engine_) {
        return Account();
    }

    return engine_->GetAccount();
}

int64_t Strategy::GetCurrentTime() const {
    if (!engine_) {
        return 0;
    }

    return engine_->GetCurrentTime();
}

std::string Strategy::GetInstrumentId() const {
    if (!engine_) {
        return "";
    }

    return engine_->GetInstrumentId();
}

double Strategy::GetTickSize() const {
    if (!engine_) {
        return 1.0;
    }

    return engine_->GetTickSize();
}

double Strategy::GetLotSize() const {
    if (!engine_) {
        return 1.0;
    }

    return engine_->GetLotSize();
}

void Strategy::Log(const std::string& message) const {
    std::cout << "[策略] " << message << std::endl;
}

} // namespace ctp_hft

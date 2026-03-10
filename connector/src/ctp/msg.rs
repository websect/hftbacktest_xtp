/// 订单响应
#[derive(Debug, Clone)]
pub struct OrderResponse {
    pub order_id: String,
    pub status: i32,
    pub filled_qty: f64,
    pub avg_price: f64,
}

/// 行情数据
#[derive(Debug, Clone)]
pub struct MarketData {
    pub symbol: String,
    pub last_price: f64,
    pub bid_price: f64,
    pub ask_price: f64,
    pub bid_qty: f64,
    pub ask_qty: f64,
    pub volume: f64,
    pub open_interest: f64,
}

/// 深度数据
#[derive(Debug, Clone)]
pub struct DepthData {
    pub symbol: String,
    pub bids: Vec<(f64, f64)>, // (price, qty)
    pub asks: Vec<(f64, f64)>, // (price, qty)
}

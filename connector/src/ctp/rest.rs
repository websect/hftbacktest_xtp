use std::time::Duration;

use hftbacktest::types::{OrderType, Side, TimeInForce};
use tokio::time::sleep;

use crate::ctp::{CtpError, msg::OrderResponse};

/// CTP REST API 客户端
pub struct CtpClient {
    broker_id: String,
    investor_id: String,
    password: String,
    app_id: String,
    auth_code: String,
}

impl CtpClient {
    pub fn new(config: &crate::ctp::Config) -> Result<Self, CtpError> {
        Ok(Self {
            broker_id: config.broker_id.clone(),
            investor_id: config.investor_id.clone(),
            password: config.password.clone(),
            app_id: config.app_id.clone(),
            auth_code: config.auth_code.clone(),
        })
    }

    /// 提交订单
    pub async fn submit_order(
        &self,
        client_order_id: &str,
        symbol: &str,
        side: Side,
        price: f64,
        price_precision: u32,
        qty: f64,
        order_type: OrderType,
        _time_in_force: TimeInForce,
    ) -> Result<OrderResponse, CtpError> {
        // 模拟网络延迟
        sleep(Duration::from_millis(10)).await;

        // 这里应该调用实际的 CTP API
        // 例如：使用 CTP C++ SDK 或 Python 绑定

        debug!(
            "Submitting order: client_order_id={}, symbol={}, side={:?}, price={}, qty={}",
            client_order_id, symbol, side, price, qty
        );

        // 模拟成功响应
        Ok(OrderResponse {
            order_id: client_order_id.to_string(),
            status: 1, // 部分成交
            filled_qty: 0.0,
            avg_price: price,
        })
    }

    /// 撤销订单
    pub async fn cancel_order(&self, client_order_id: &str, symbol: &str) -> Result<OrderResponse, CtpError> {
        // 模拟网络延迟
        sleep(Duration::from_millis(10)).await;

        debug!("Canceling order: client_order_id={}, symbol={}", client_order_id, symbol);

        // 模拟成功响应
        Ok(OrderResponse {
            order_id: client_order_id.to_string(),
            status: 3, // 已撤单
            filled_qty: 0.0,
            avg_price: 0.0,
        })
    }

    /// 查询订单
    pub async fn query_order(&self, client_order_id: &str, symbol: &str) -> Result<OrderResponse, CtpError> {
        debug!("Querying order: client_order_id={}, symbol={}", client_order_id, symbol);

        // 模拟响应
        Ok(OrderResponse {
            order_id: client_order_id.to_string(),
            status: 1, // 部分成交
            filled_qty: 0.0,
            avg_price: 0.0,
        })
    }
}

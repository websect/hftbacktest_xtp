use std::{
    collections::HashMap,
    sync::{Arc, Mutex},
};

use hftbacktest::types::{Order, Status};

use crate::ctp::CtpError;

/// CTP 订单管理器
pub struct CtpOrderManager {
    order_prefix: String,
    orders: Arc<Mutex<HashMap<String, Order>>>,
    client_order_ids: Arc<Mutex<HashMap<String, String>>>, // order_id -> client_order_id
    order_id_counter: Arc<Mutex<u64>>,
}

impl CtpOrderManager {
    pub fn new(order_prefix: &str) -> Self {
        Self {
            order_prefix: order_prefix.to_string(),
            orders: Arc::new(Mutex::new(HashMap::new())),
            client_order_ids: Arc::new(Mutex::new(HashMap::new())),
            order_id_counter: Arc::new(Mutex::new(0)),
        }
    }

    /// 生成客户端订单ID
    pub fn prepare_client_order_id(&self, symbol: String, order: Order) -> Option<String> {
        let order_id = order.order_id;
        let client_order_id = format!("{}_{}_{}", self.order_prefix, symbol, order_id);

        // 检查是否已存在
        let mut client_order_ids = self.client_order_ids.lock().unwrap();
        if client_order_ids.contains_key(&order_id.to_string()) {
            return None; // 重复的订单ID
        }

        client_order_ids.insert(order_id.to_string(), client_order_id.clone());

        // 保存订单
        let mut orders = self.orders.lock().unwrap();
        orders.insert(order_id.to_string(), order);

        Some(client_order_id)
    }

    /// 根据订单ID获取客户端订单ID
    pub fn get_client_order_id(&self, symbol: &str, order_id: u64) -> Option<String> {
        let client_order_ids = self.client_order_ids.lock().unwrap();
        client_order_ids.get(&order_id.to_string()).cloned()
    }

    /// 更新订单状态（从REST响应）
    pub fn update_from_rest(&self, client_order_id: &str, resp: &crate::ctp::msg::OrderResponse) -> Option<Order> {
        // 解析客户端订单ID获取原始订单ID
        let order_id = self.extract_order_id(client_order_id)?;

        let mut orders = self.orders.lock().unwrap();
        if let Some(order) = orders.get_mut(&order_id) {
            // 更新订单状态
            order.req = Status::Resting;
            order.status = self.map_status(resp.status);

            return Some(order.clone());
        }

        None
    }

    /// 订单提交失败
    pub fn update_submit_fail(&self, client_order_id: &str, error: &CtpError) -> Option<Order> {
        let order_id = self.extract_order_id(client_order_id)?;

        let mut orders = self.orders.lock().unwrap();
        if let Some(order) = orders.get_mut(&order_id) {
            order.req = Status::None;
            order.status = Status::Expired;

            return Some(order.clone());
        }

        None
    }

    /// 订单撤单失败
    pub fn update_cancel_fail(&self, client_order_id: &str, error: &CtpError) -> Option<Order> {
        let order_id = self.extract_order_id(client_order_id)?;

        let mut orders = self.orders.lock().unwrap();
        if let Some(order) = orders.get_mut(&order_id) {
            // 保持订单状态不变
            return Some(order.clone());
        }

        None
    }

    /// 获取当前订单
    pub fn orders(&self, symbol: Option<String>) -> Vec<Order> {
        let orders = self.orders.lock().unwrap();
        if let Some(symbol) = symbol {
            orders
                .values()
                .filter(|order| order.symbol == symbol)
                .cloned()
                .collect()
        } else {
            orders.values().cloned().collect()
        }
    }

    /// 从客户端订单ID提取原始订单ID
    fn extract_order_id(&self, client_order_id: &str) -> Option<String> {
        // 格式: prefix_symbol_orderId
        let parts: Vec<&str> = client_order_id.split('_').collect();
        if parts.len() >= 3 {
            Some(parts[parts.len() - 1].to_string())
        } else {
            None
        }
    }

    /// 映射 CTP 订单状态到 hftbacktest 状态
    fn map_status(&self, ctp_status: i32) -> Status {
        match ctp_status {
            0 => Status::New,           // 全部成交
            1 => Status::Resting,       // 部分成交
            3 => Status::Canceled,      // 已撤单
            4 => Status::Expired,       // 已拒绝
            5 => Status::Expired,       // 已撤单
            _ => Status::Expired,       // 未知状态
        }
    }
}

impl crate::connector::GetOrders for CtpOrderManager {
    fn orders(&self, symbol: Option<String>) -> Vec<Order> {
        self.orders(symbol)
    }
}

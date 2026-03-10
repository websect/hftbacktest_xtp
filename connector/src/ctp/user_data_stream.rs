use std::sync::Arc;

use hftbacktest::types::{LiveEvent, Order};
use tokio::sync::mpsc::UnboundedSender;
use tracing::debug;

use crate::ctp::{CtpError, PublishEvent};

/// CTP 用户数据流处理
pub struct CtpUserDataStream {
    client: Arc<Mutex<crate::ctp::rest::CtpClient>>,
    ev_tx: UnboundedSender<PublishEvent>,
    order_manager: Arc<Mutex<dyn crate::connector::GetOrders + Send + 'static>>,
    instruments: Arc<Mutex<std::collections::HashSet<String>>>,
    symbol_rx: tokio::sync::broadcast::Receiver<String>,
}

impl CtpUserDataStream {
    pub fn new(
        client: Arc<Mutex<crate::ctp::rest::CtpClient>>,
        ev_tx: UnboundedSender<PublishEvent>,
        order_manager: Arc<Mutex<dyn crate::connector::GetOrders + Send + 'static>>,
        instruments: Arc<Mutex<std::collections::HashSet<String>>>,
        symbol_rx: tokio::sync::broadcast::Receiver<String>,
    ) -> Self {
        Self {
            client,
            ev_tx,
            order_manager,
            instruments,
            symbol_rx,
        }
    }

    pub async fn connect(&mut self, _front_td: &str) -> Result<(), CtpError> {
        debug!("Starting CTP user data stream connection...");

        // 订阅用户数据
        while let Ok(symbol) = self.symbol_rx.recv().await {
            debug!("Subscribing to user data for symbol: {}", symbol);

            // 这里应该调用 CTP API 订阅用户数据
            // 例如: self.client.subscribe_user_data(&symbol).await?;

            // 模拟接收用户数据
            self.process_user_data(&symbol).await?;
        }

        Ok(())
    }

    async fn process_user_data(&self, symbol: &str) -> Result<(), CtpError> {
        debug!("Processing user data for symbol: {}", symbol);

        // 这里处理 CTP 用户数据
        // 包括订单回报、成交回报、持仓查询等

        // 示例：发送订单事件
        // self.ev_tx.send(PublishEvent::LiveEvent(LiveEvent::Order {
        //     symbol: symbol.to_string(),
        //     order: ...,
        // }))?;

        Ok(())
    }
}

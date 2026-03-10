use std::sync::Arc;

use hftbacktest::types::{LiveEvent, Order};
use tokio::sync::mpsc::UnboundedSender;
use tracing::debug;

use crate::ctp::{CtpError, PublishEvent};

/// CTP 行情数据流处理
pub struct CtpMarketDataStream {
    client: Arc<Mutex<crate::ctp::rest::CtpClient>>,
    ev_tx: UnboundedSender<PublishEvent>,
    symbol_rx: tokio::sync::broadcast::Receiver<String>,
}

impl CtpMarketDataStream {
    pub fn new(
        client: Arc<Mutex<crate::ctp::rest::CtpClient>>,
        ev_tx: UnboundedSender<PublishEvent>,
        symbol_rx: tokio::sync::broadcast::Receiver<String>,
    ) -> Self {
        Self {
            client,
            ev_tx,
            symbol_rx,
        }
    }

    pub async fn connect(&mut self, _front_md: &str) -> Result<(), CtpError> {
        debug!("Starting CTP market data stream connection...");

        // 订阅行情数据
        while let Ok(symbol) = self.symbol_rx.recv().await {
            debug!("Subscribing to market data for symbol: {}", symbol);

            // 这里应该调用 CTP API 订阅行情
            // 例如: self.client.subscribe_market_data(&symbol).await?;

            // 模拟接收行情数据
            self.process_market_data(&symbol).await?;
        }

        Ok(())
    }

    async fn process_market_data(&self, symbol: &str) -> Result<(), CtpError> {
        // 这里处理 CTP 行情数据
        // 包括深度数据、成交数据等

        debug!("Processing market data for symbol: {}", symbol);

        // 示例：发送深度数据事件
        // self.ev_tx.send(PublishEvent::LiveEvent(LiveEvent::MarketDepth {
        //     symbol: symbol.to_string(),
        //     depth: ...,
        // }))?;

        Ok(())
    }
}

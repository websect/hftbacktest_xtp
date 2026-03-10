mod market_data_stream;
mod msg;
mod order_manager;
mod rest;
mod user_data_stream;

use std::{
    collections::{HashMap, HashSet},
    sync::{Arc, Mutex},
};

use hftbacktest::{
    prelude::get_precision,
    types::{ErrorKind, LiveError, LiveEvent, Order, Status, Value},
};
use serde::Deserialize;
use thiserror::Error;
use tokio::sync::{broadcast, broadcast::Sender, mpsc::UnboundedSender};
use tracing::{debug, error, warn};

use crate::{
    connector::{Connector, ConnectorBuilder, GetOrders, PublishEvent},
    utils::{ExponentialBackoff, Retry},
};

#[derive(Error, Debug)]
pub enum CtpError {
    #[error("InstrumentNotFound")]
    InstrumentNotFound,
    #[error("InvalidRequest")]
    InvalidRequest,
    #[error("ConnectionInterrupted")]
    ConnectionInterrupted,
    #[error("ConnectionAbort: {0}")]
    ConnectionAbort(String),
    #[error("ReqError: {0:?}")]
    ReqError(String),
    #[error("OrderError: {code} - {msg}")]
    OrderError { code: i32, msg: String },
    #[error("PrefixUnmatched")]
    PrefixUnmatched,
    #[error("OrderNotFound")]
    OrderNotFound,
    #[error("Config: {0:?}")]
    Config(#[from] toml::de::Error),
    #[error("CtpApiError: {0}")]
    CtpApiError(String),
}

impl From<CtpError> for Value {
    fn from(value: CtpError) -> Value {
        match value {
            CtpError::InstrumentNotFound => Value::String(value.to_string()),
            CtpError::InvalidRequest => Value::String(value.to_string()),
            CtpError::ReqError(error) => {
                let mut map = HashMap::new();
                map.insert("msg".to_string(), Value::String(error));
                Value::Map(map)
            }
            CtpError::OrderError { code, msg } => Value::Map({
                let mut map = HashMap::new();
                map.insert("code".to_string(), Value::Int(code as i64));
                map.insert("msg".to_string(), Value::String(msg));
                map
            }),
            CtpError::ConnectionInterrupted => Value::String(value.to_string()),
            CtpError::ConnectionAbort(_) => Value::String(value.to_string()),
            CtpError::Config(_) => Value::String(value.to_string()),
            CtpError::PrefixUnmatched => Value::String(value.to_string()),
            CtpError::OrderNotFound => Value::String(value.to_string()),
            CtpError::CtpApiError(error) => Value::String(error),
        }
    }
}

#[derive(Deserialize, Clone)]
pub struct Config {
    #[serde(default)]
    pub broker_id: String,
    #[serde(default)]
    pub investor_id: String,
    #[serde(default)]
    pub password: String,
    #[serde(default)]
    pub app_id: String,
    #[serde(default)]
    pub auth_code: String,
    #[serde(default)]
    pub front_md: String,
    #[serde(default)]
    pub front_td: String,
    #[serde(default)]
    pub order_prefix: String,
}

type SharedSymbolSet = Arc<Mutex<HashSet<String>>>;

/// A connector for CTP (China Futures Trading Protocol).
pub struct Ctp {
    config: Config,
    symbols: SharedSymbolSet,
    order_manager: Arc<Mutex<dyn GetOrders + Send + 'static>>,
    symbol_tx: Sender<String>,
    client: Arc<Mutex<crate::ctp::rest::CtpClient>>,
}

impl Ctp {
    pub fn new(config: Config, order_manager: Arc<Mutex<dyn GetOrders + Send + 'static>>) -> Result<Self, CtpError> {
        let client = Arc::new(Mutex::new(crate::ctp::rest::CtpClient::new(&config)?));
        let (symbol_tx, _) = broadcast::channel(500);

        Ok(Ctp {
            config,
            symbols: Default::default(),
            order_manager,
            symbol_tx,
            client,
        })
    }

    pub fn connect_market_data_stream(&mut self, ev_tx: UnboundedSender<PublishEvent>) {
        let base_url = self.config.front_md.clone();
        let client = self.client.clone();
        let symbol_tx = self.symbol_tx.clone();

        tokio::spawn(async move {
            let _ = Retry::new(ExponentialBackoff::default())
                .error_handler(|error: CtpError| {
                    error!(
                        ?error,
                        "An error occurred in the CTP market data stream connection."
                    );
                    ev_tx
                        .send(PublishEvent::LiveEvent(LiveEvent::Error(LiveError::with(
                            ErrorKind::ConnectionInterrupted,
                            error.into(),
                        ))))
                        .unwrap();
                    Ok(())
                })
                .retry(|| async {
                    let mut stream = market_data_stream::CtpMarketDataStream::new(
                        client.clone(),
                        ev_tx.clone(),
                        symbol_tx.subscribe(),
                    );
                    debug!("Connecting to the CTP market data stream...");
                    stream.connect(&base_url).await?;
                    debug!("The CTP market data stream connection is permanently closed.");
                    Ok(())
                })
                .await;
        });
    }

    pub fn connect_user_data_stream(&self, ev_tx: UnboundedSender<PublishEvent>) {
        let base_url = self.config.front_td.clone();
        let client = self.client.clone();
        let order_manager = self.order_manager.clone();
        let instruments = self.symbols.clone();
        let symbol_tx = self.symbol_tx.clone();

        tokio::spawn(async move {
            let _ = Retry::new(ExponentialBackoff::default())
                .error_handler(|error: CtpError| {
                    error!(
                        ?error,
                        "An error occurred in the CTP user data stream connection."
                    );
                    ev_tx
                        .send(PublishEvent::LiveEvent(LiveEvent::Error(LiveError::with(
                            ErrorKind::ConnectionInterrupted,
                            error.into(),
                        ))))
                        .unwrap();
                    Ok(())
                })
                .retry(|| async {
                    let mut stream = user_data_stream::CtpUserDataStream::new(
                        client.clone(),
                        ev_tx.clone(),
                        order_manager.clone(),
                        instruments.clone(),
                        symbol_tx.subscribe(),
                    );

                    debug!("Connecting to the CTP user data stream...");
                    stream.connect(&base_url).await?;
                    debug!("The CTP user data stream connection is permanently closed.");
                    Ok(())
                })
                .await;
        });
    }
}

impl ConnectorBuilder for Ctp {
    type Error = CtpError;

    fn build_from(config: &str) -> Result<Self, Self::Error> {
        let config: Config = toml::from_str(config)?;

        let order_manager = Arc::new(Mutex::new(order_manager::CtpOrderManager::new(&config.order_prefix)));

        Ctp::new(config, order_manager)
    }
}

impl Connector for Ctp {
    fn register(&mut self, symbol: String) {
        let mut symbols = self.symbols.lock().unwrap();
        if !symbols.contains(&symbol) {
            symbols.insert(symbol.clone());
            self.symbol_tx.send(symbol).unwrap();
        }
    }

    fn order_manager(&self) -> Arc<Mutex<dyn GetOrders + Send + 'static>> {
        self.order_manager.clone()
    }

    fn run(&mut self, ev_tx: UnboundedSender<PublishEvent>) {
        self.connect_market_data_stream(ev_tx.clone());
        self.connect_user_data_stream(ev_tx.clone());
    }

    fn submit(&self, symbol: String, mut order: Order, tx: UnboundedSender<PublishEvent>) {
        let client = self.client.clone();
        let order_manager = self.order_manager.clone();

        tokio::spawn(async move {
            // 获取订单管理器的具体类型
            let order_manager_ref = order_manager.lock().unwrap();
            if let Some(ctp_order_manager) = order_manager_ref.downcast_ref::<order_manager::CtpOrderManager>() {
                let client_order_id = ctp_order_manager.prepare_client_order_id(symbol.clone(), order.clone());

                match client_order_id {
                    Some(client_order_id) => {
                        let result = client.lock().unwrap()
                            .submit_order(
                                &client_order_id,
                                &symbol,
                                order.side,
                                order.price_tick as f64 * order.tick_size,
                                get_precision(order.tick_size),
                                order.qty,
                                order.order_type,
                                order.time_in_force,
                            )
                            .await;

                        match result {
                            Ok(resp) => {
                                if let Some(order) = ctp_order_manager.update_from_rest(&client_order_id, &resp) {
                                    tx.send(PublishEvent::LiveEvent(LiveEvent::Order {
                                        symbol,
                                        order,
                                    }))
                                    .unwrap();
                                }
                            }
                            Err(error) => {
                                if let Some(order) = ctp_order_manager.update_submit_fail(&client_order_id, &error) {
                                    tx.send(PublishEvent::LiveEvent(LiveEvent::Order {
                                        symbol,
                                        order,
                                    }))
                                    .unwrap();
                                }

                                tx.send(PublishEvent::LiveEvent(LiveEvent::Error(LiveError::with(
                                    ErrorKind::OrderError,
                                    error.into(),
                                ))))
                                .unwrap();
                            }
                        }
                    }
                    None => {
                        warn!(
                            ?order,
                            "Coincidentally, creates a duplicated client order id. \
                            This order request will be expired."
                        );
                        order.req = Status::None;
                        order.status = Status::Expired;
                        tx.send(PublishEvent::LiveEvent(LiveEvent::Order { symbol, order }))
                            .unwrap();
                    }
                }
            }
        });
    }

    fn cancel(&self, symbol: String, order: Order, tx: UnboundedSender<PublishEvent>) {
        let client = self.client.clone();
        let order_manager = self.order_manager.clone();

        tokio::spawn(async move {
            let order_manager_ref = order_manager.lock().unwrap();
            if let Some(ctp_order_manager) = order_manager_ref.downcast_ref::<order_manager::CtpOrderManager>() {
                let client_order_id = ctp_order_manager.get_client_order_id(&symbol, order.order_id);

                match client_order_id {
                    Some(client_order_id) => {
                        let result = client.lock().unwrap().cancel_order(&client_order_id, &symbol).await;

                        match result {
                            Ok(resp) => {
                                if let Some(order) = ctp_order_manager.update_from_rest(&client_order_id, &resp) {
                                    tx.send(PublishEvent::LiveEvent(LiveEvent::Order {
                                        symbol,
                                        order,
                                    }))
                                    .unwrap();
                                }
                            }
                            Err(error) => {
                                if let Some(order) = ctp_order_manager.update_cancel_fail(&client_order_id, &error) {
                                    tx.send(PublishEvent::LiveEvent(LiveEvent::Order {
                                        symbol,
                                        order,
                                    }))
                                    .unwrap();
                                }

                                tx.send(PublishEvent::LiveEvent(LiveEvent::Error(LiveError::with(
                                    ErrorKind::OrderError,
                                    error.into(),
                                ))))
                                .unwrap();
                            }
                        }
                    }
                    None => {
                        warn!(
                            order_id = order.order_id,
                            "client_order_id corresponding to order_id is not found; \
                            this may be due to the order already being canceled or filled."
                        );
                    }
                }
            }
        });
    }
}

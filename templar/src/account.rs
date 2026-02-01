//! Account module for Templar.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::sync::mpsc;

use bwk_sp::{Account as SpAccount, AccountError, Notification as SpNotification};

use crate::config::Config;
use crate::ffi::{CoinState, Notification, NotificationFlag, RustCoin, RustTx};

/// Account wrapping bwk-sp::Account.
pub struct Account {
    /// Inner bwk-sp Account
    inner: SpAccount,
    /// Notification receiver
    receiver: Option<mpsc::Receiver<SpNotification>>,
}

impl Account {
    /// Create a new Account from config.
    pub fn new(config: Config) -> Result<Self, AccountError> {
        let sp_config = config.to_sp_config();
        let mut inner = SpAccount::new(sp_config)?;
        let receiver = inner.receiver();

        Ok(Account { inner, receiver })
    }

    /// Start the scanner.
    pub fn start_scanner(&mut self) -> Result<(), String> {
        self.inner
            .start_scanner()
            .map_err(|e| format!("scanner error: {e}"))
    }

    /// Stop the scanner.
    pub fn stop_scanner(&mut self) {
        self.inner.stop_scanner()
    }

    /// Try to receive a notification (non-blocking).
    pub fn try_recv(&mut self) -> Box<Poll> {
        if let Some(ref receiver) = self.receiver {
            match receiver.try_recv() {
                Ok(notif) => {
                    let notification = convert_notification(notif);
                    Box::new(Poll::ok(notification))
                }
                Err(mpsc::TryRecvError::Empty) => Box::new(Poll::none()),
                Err(mpsc::TryRecvError::Disconnected) => {
                    Box::new(Poll::err("Notification channel disconnected"))
                }
            }
        } else {
            Box::new(Poll::err("No notification receiver"))
        }
    }

    /// Get account name.
    pub fn name(&self) -> String {
        self.inner.name().to_string()
    }

    /// Get balance in satoshis.
    pub fn balance(&self) -> u64 {
        self.inner.balance()
    }

    /// Get the Silent Payment address for this account.
    pub fn sp_address(&self) -> String {
        self.inner.sp_address().to_string()
    }

    /// Get all coins (both spent and unspent).
    pub fn coins(&self) -> Vec<RustCoin> {
        self.inner
            .coins()
            .into_iter()
            .map(|(outpoint, entry)| RustCoin {
                outpoint: format!("{}:{}", outpoint.txid, outpoint.vout),
                value: entry.amount_sat(),
                height: entry.height(),
                label: self.inner.get_coin_label(&outpoint).unwrap_or_default(),
                spent: !entry.is_spendable(),
            })
            .collect()
    }

    /// Get spendable coins summary.
    pub fn spendable_coins(&self) -> CoinState {
        let coin_state = self.inner.spendable_coins();
        CoinState {
            confirmed_count: coin_state.confirmed_coins as u64,
            confirmed_balance: coin_state.confirmed_balance,
            unconfirmed_count: coin_state.unconfirmed_coins as u64,
            unconfirmed_balance: coin_state.unconfirmed_balance,
        }
    }

    /// Update the label for a coin.
    pub fn update_coin_label(&mut self, outpoint: String, label: String) -> Result<(), String> {
        // Parse outpoint from "txid:vout" format
        let parts: Vec<&str> = outpoint.split(':').collect();
        if parts.len() != 2 {
            return Err("Invalid outpoint format, expected 'txid:vout'".to_string());
        }

        let txid = parts[0]
            .parse::<bitcoin::Txid>()
            .map_err(|e| format!("Invalid txid: {e}"))?;
        let vout = parts[1]
            .parse::<u32>()
            .map_err(|e| format!("Invalid vout: {e}"))?;

        let outpoint = bitcoin::OutPoint { txid, vout };
        self.inner.update_coin_label(outpoint, label);
        Ok(())
    }

    /// Get payment history.
    pub fn payment_history(&self) -> Vec<RustTx> {
        self.inner
            .payment_history()
            .into_iter()
            .map(|payment| {
                let direction = match payment.payment_type {
                    bwk_sp::PaymentType::Receive => "incoming",
                    bwk_sp::PaymentType::Send => "outgoing",
                };

                RustTx {
                    txid: payment.txid,
                    direction: direction.to_string(),
                    amount: payment.amount,
                    fee: 0, // Fee information is not available in Payment struct
                    height: payment.height.unwrap_or(0), // 0 for unconfirmed
                }
            })
            .collect()
    }
}

/// Convert bwk-sp::Notification to Templar Notification.
fn convert_notification(sp_notif: SpNotification) -> Notification {
    match sp_notif {
        SpNotification::ScanStarted => Notification {
            flag: NotificationFlag::ScanStarted,
            payload: String::new(),
        },
        SpNotification::ScanProgress { current, end } => Notification {
            flag: NotificationFlag::ScanProgress,
            payload: format!("{current}/{end}"),
        },
        SpNotification::ScanCompleted => Notification {
            flag: NotificationFlag::ScanCompleted,
            payload: String::new(),
        },
        SpNotification::ScanError {
            message,
            retries_attempted,
        } => Notification {
            flag: NotificationFlag::ScanError,
            payload: format!("{message} (retries: {retries_attempted})"),
        },
        SpNotification::NewOutput(outpoint) => Notification {
            flag: NotificationFlag::NewOutput,
            payload: outpoint.to_string(),
        },
        SpNotification::OutputSpent(outpoint) => Notification {
            flag: NotificationFlag::OutputSpent,
            payload: outpoint.to_string(),
        },
        SpNotification::Stopped => Notification {
            flag: NotificationFlag::Stopped,
            payload: String::new(),
        },
    }
}

// CXX FFI functions

/// Create a new account.
pub fn new_account(account_name: String) -> Result<Box<Account>, String> {
    let config = crate::config::Config::from_file(account_name)
        .map_err(|e| format!("failed to load config: {e}"))?;

    Account::new(config)
        .map(Box::new)
        .map_err(|e| {
            log::error!("Failed to create account: {e}");
            format!("Failed to create account: {e}")
        })
}

/// Poll result wrapper for CXX.
pub struct Poll {
    is_some: bool,
    notification: Notification,
    error: String,
}

impl Poll {
    /// Create a Poll with a notification.
    pub fn ok(notification: Notification) -> Self {
        Poll {
            is_some: true,
            notification,
            error: String::new(),
        }
    }

    /// Create an empty Poll.
    pub fn none() -> Self {
        Poll {
            is_some: false,
            notification: Notification {
                flag: NotificationFlag::Stopped,
                payload: String::new(),
            },
            error: String::new(),
        }
    }

    /// Create a Poll with an error.
    pub fn err(error: &str) -> Self {
        Poll {
            is_some: false,
            notification: Notification {
                flag: NotificationFlag::ScanError,
                payload: String::new(),
            },
            error: error.to_string(),
        }
    }

    /// Check if poll has a notification.
    pub fn is_some(&self) -> bool {
        self.is_some
    }

    /// Get the notification.
    pub fn get_notification(&self) -> Notification {
        self.notification.clone()
    }

    /// Get error message.
    pub fn get_error(&self) -> String {
        self.error.clone()
    }
}

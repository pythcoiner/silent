//! Account module for Templar.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::sync::mpsc;

use bwk_sp::{Account as SpAccount, AccountError, Notification as SpNotification};

use crate::config::Config;
use crate::ffi::{Notification, NotificationFlag};

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
    pub fn start_scanner(&mut self) -> Result<(), AccountError> {
        self.inner.start_scanner()
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
pub fn new_account(account_name: String) -> Box<Account> {
    let config = crate::config::Config::from_file(account_name)
        .expect("failed to load config");

    match Account::new(config) {
        Ok(account) => Box::new(account),
        Err(e) => {
            log::error!("Failed to create account: {e}");
            panic!("Failed to create account: {e}");
        }
    }
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

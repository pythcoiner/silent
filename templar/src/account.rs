//! Account module for Templar.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::sync::mpsc;

use bwk_sp::{Account as SpAccount, AccountError, Notification as SpNotification};
use spdk_core::{RecipientAddress, SilentPaymentUnsignedTransaction};

use crate::config::Config;
use crate::ffi::{CoinState, Notification, NotificationFlag, RustCoin, RustTx, TransactionSimulation, TransactionTemplate};

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

    /// Simulate a transaction without actually creating it.
    pub fn simulate_transaction(&self, tx_template: TransactionTemplate) -> TransactionSimulation {
        use bitcoin::Amount;
        use spdk_core::FeeRate;

        // Parse outputs and check for send_max flag
        let mut recipients = Vec::new();
        let mut has_send_max = false;
        let mut send_max_addr = None;

        for output in tx_template.outputs {
            // Parse address
            let addr = match RecipientAddress::try_from(output.address.clone()) {
                Ok(a) => a,
                Err(e) => {
                    return TransactionSimulation {
                        is_valid: false,
                        fee: 0,
                        weight: 0,
                        input_total: 0,
                        output_total: 0,
                        input_count: 0,
                        error: format!("Invalid address '{}': {}", output.address, e),
                    };
                }
            };

            if output.send_max {
                if has_send_max {
                    return TransactionSimulation {
                        is_valid: false,
                        fee: 0,
                        weight: 0,
                        input_total: 0,
                        output_total: 0,
                        input_count: 0,
                        error: String::from("Only one output can have send_max=true"),
                    };
                }
                has_send_max = true;
                send_max_addr = Some(addr);
            } else {
                recipients.push((addr, Amount::from_sat(output.amount)));
            }
        }

        // Convert fee rate (ensure it's at least 1.0 sat/vb)
        let fee_rate = FeeRate::from_sat_per_vb(tx_template.fee_rate.max(1.0) as f32);

        // Create the unsigned transaction
        let unsigned_tx_result = if has_send_max {
            if !recipients.is_empty() {
                return TransactionSimulation {
                    is_valid: false,
                    fee: 0,
                    weight: 0,
                    input_total: 0,
                    output_total: 0,
                    input_count: 0,
                    error: String::from("send_max cannot be combined with other outputs"),
                };
            }
            self.inner.create_drain_transaction(send_max_addr.unwrap(), fee_rate)
        } else {
            self.inner.create_transaction(recipients, fee_rate)
        };

        // Extract simulation data
        match unsigned_tx_result {
            Ok(unsigned_tx) => {
                let input_total: u64 = unsigned_tx.selected_utxos.iter()
                    .map(|(_, owned)| owned.amount.to_sat())
                    .sum();
                let output_total: u64 = unsigned_tx.recipients.iter()
                    .map(|r| r.amount.to_sat())
                    .sum();
                let fee = input_total.saturating_sub(output_total);
                let input_count = unsigned_tx.selected_utxos.len() as u64;

                // Estimate weight from unsigned_tx if available
                let weight = if let Some(ref tx) = unsigned_tx.unsigned_tx {
                    tx.weight().to_wu()
                } else {
                    // Rough estimate: 4 * vsize, where vsize ≈ fee / fee_rate
                    let vbytes = (fee as f64 / tx_template.fee_rate).ceil() as u64;
                    vbytes * 4
                };

                TransactionSimulation {
                    is_valid: true,
                    fee,
                    weight,
                    input_total,
                    output_total,
                    input_count,
                    error: String::new(),
                }
            }
            Err(e) => {
                TransactionSimulation {
                    is_valid: false,
                    fee: 0,
                    weight: 0,
                    input_total: 0,
                    output_total: 0,
                    input_count: 0,
                    error: e.to_string(),
                }
            }
        }
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

/// PsbtResult wrapper for CXX.
pub struct PsbtResult {
    inner: Result<SilentPaymentUnsignedTransaction, String>,
}

impl PsbtResult {
    /// Check if result is valid.
    pub fn is_ok(&self) -> bool {
        self.inner.is_ok()
    }

    /// Get error message (only valid if !is_ok()).
    pub fn get_psbt_error(&self) -> String {
        match &self.inner {
            Ok(_) => String::new(),
            Err(e) => e.clone(),
        }
    }

    /// Get transaction ID preview (only valid if is_ok()).
    pub fn get_txid_preview(&self) -> String {
        match &self.inner {
            Ok(unsigned_tx) => {
                if let Some(ref tx) = unsigned_tx.unsigned_tx {
                    tx.compute_txid().to_string()
                } else {
                    String::from("[not finalized]")
                }
            }
            Err(_) => String::new(),
        }
    }

    /// Internal: get the unsigned transaction (for signing later)
    #[allow(dead_code)]
    pub(crate) fn into_inner(self) -> Result<SilentPaymentUnsignedTransaction, String> {
        self.inner
    }
}

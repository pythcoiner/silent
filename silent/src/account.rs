//! Account module for Silent.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::sync::mpsc;

use bwk_sp::spdk_core::{
    self, bip39, RecipientAddress, SilentPaymentUnsignedTransaction, SpClient,
};
use bwk_sp::{Account as SpAccount, AccountError, Fees, Notification as SpNotification, ScanMode};

use crate::config::Config;
use crate::ffi::{
    CoinState, Notification, NotificationFlag, RustCoin, RustTx, TransactionSimulation,
    TransactionTemplate, TxResult,
};

/// Valid account state (created successfully).
struct AccountInner {
    account: SpAccount,
    receiver: Option<mpsc::Receiver<SpNotification>>,
    client: SpClient,
}

/// Account wrapping bwk-sp::Account.
/// Always constructible; check is_ok() before use.
pub struct Account {
    inner: Option<AccountInner>,
    error: String,
}

impl Account {
    /// Create a new Account from config (for Rust-side use / tests).
    pub fn new(config: Config) -> Result<Self, AccountError> {
        let inner = Self::new_inner(config)?;
        Ok(Account {
            inner: Some(inner),
            error: String::new(),
        })
    }

    /// Create a new AccountInner from config.
    fn new_inner(config: Config) -> Result<AccountInner, AccountError> {
        let sp_config = config.to_sp_config();
        let mut account = SpAccount::new(sp_config)?;
        let receiver = account.receiver();

        // Create SpClient from mnemonic for manual transaction creation
        let mnemonic = bip39::Mnemonic::parse(&config.mnemonic)
            .map_err(|e| AccountError::Config(format!("Invalid mnemonic: {e}")))?;
        let network = match config.network {
            crate::ffi::Network::Regtest => bitcoin::Network::Regtest,
            crate::ffi::Network::Signet => bitcoin::Network::Signet,
            crate::ffi::Network::Testnet => bitcoin::Network::Testnet,
            crate::ffi::Network::Bitcoin => bitcoin::Network::Bitcoin,
            _ => unreachable!("Invalid network variant"),
        };
        let client = SpClient::new_from_mnemonic(mnemonic, network)
            .map_err(|e| AccountError::Config(format!("Failed to create SpClient: {e}")))?;

        Ok(AccountInner {
            account,
            receiver,
            client,
        })
    }

    /// Check if the account was created successfully.
    pub fn is_ok(&self) -> bool {
        self.inner.is_some()
    }

    /// Get error message (empty if is_ok).
    pub fn get_error(&self) -> String {
        self.error.clone()
    }

    /// Start the scanner in continuous mode.
    pub fn start_scanner(&mut self) -> bool {
        let Some(inner) = &mut self.inner else {
            return false;
        };
        match inner.account.start_scan(ScanMode::Continuous) {
            Ok(()) => true,
            Err(e) => {
                log::error!("scanner error: {e}");
                false
            }
        }
    }

    /// Stop the scanner.
    pub fn stop_scanner(&mut self) {
        if let Some(inner) = &mut self.inner {
            inner.account.stop_scan();
        }
    }

    /// Try to receive a notification (non-blocking).
    pub fn try_recv(&mut self) -> Box<Poll> {
        let Some(inner) = &mut self.inner else {
            return Box::new(Poll::err("Account not initialized"));
        };
        if let Some(ref receiver) = inner.receiver {
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
        match &self.inner {
            Some(inner) => inner.account.name().to_string(),
            None => String::new(),
        }
    }

    /// Get balance in satoshis.
    pub fn balance(&self) -> u64 {
        match &self.inner {
            Some(inner) => inner.account.balance(),
            None => 0,
        }
    }

    /// Get the Silent Payment address for this account.
    pub fn sp_address(&self) -> String {
        match &self.inner {
            Some(inner) => inner.account.sp_address().to_string(),
            None => String::new(),
        }
    }

    /// Get all coins (both spent and unspent).
    pub fn coins(&self) -> Vec<RustCoin> {
        let Some(inner) = &self.inner else {
            return vec![];
        };
        inner
            .account
            .coins()
            .into_iter()
            .map(|(outpoint, entry)| RustCoin {
                outpoint: format!("{}:{}", outpoint.txid, outpoint.vout),
                value: entry.amount_sat(),
                height: entry.height(),
                label: inner.account.get_coin_label(&outpoint).unwrap_or_default(),
                spent: !entry.is_spendable(),
            })
            .collect()
    }

    /// Get spendable coins summary.
    pub fn spendable_coins(&self) -> CoinState {
        match &self.inner {
            Some(inner) => {
                let coin_state = inner.account.spendable_coins();
                CoinState {
                    confirmed_count: coin_state.confirmed_coins as u64,
                    confirmed_balance: coin_state.confirmed_balance,
                    unconfirmed_count: coin_state.unconfirmed_coins as u64,
                    unconfirmed_balance: coin_state.unconfirmed_balance,
                }
            }
            None => CoinState {
                confirmed_count: 0,
                confirmed_balance: 0,
                unconfirmed_count: 0,
                unconfirmed_balance: 0,
            },
        }
    }

    /// Update the label for a coin.
    pub fn update_coin_label(&mut self, outpoint: String, label: String) -> bool {
        let Some(inner) = &mut self.inner else {
            return false;
        };
        // Parse outpoint from "txid:vout" format
        let parts: Vec<&str> = outpoint.split(':').collect();
        if parts.len() != 2 {
            log::error!("Invalid outpoint format, expected 'txid:vout'");
            return false;
        }

        let Ok(txid) = parts[0].parse::<bitcoin::Txid>() else {
            log::error!("Invalid txid: {}", parts[0]);
            return false;
        };
        let Ok(vout) = parts[1].parse::<u32>() else {
            log::error!("Invalid vout: {}", parts[1]);
            return false;
        };

        let outpoint = bitcoin::OutPoint { txid, vout };
        inner.account.update_coin_label(outpoint, label);
        true
    }

    /// Get payment history.
    pub fn payment_history(&self) -> Vec<RustTx> {
        let Some(inner) = &self.inner else {
            return vec![];
        };
        inner
            .account
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
                    fee: 0,
                    height: payment.height.unwrap_or(0),
                }
            })
            .collect()
    }

    /// Create transaction with manual coin selection.
    fn create_transaction_with_manual_selection(
        inner: &AccountInner,
        input_outpoints: Vec<String>,
        recipients: Vec<(RecipientAddress, bitcoin::Amount)>,
        max_addr: Option<RecipientAddress>,
        has_max: bool,
        fee_rate: spdk_core::FeeRate,
    ) -> Result<SilentPaymentUnsignedTransaction, AccountError> {
        use bitcoin::OutPoint;
        use bwk_sp::spdk_core::OwnedOutput;

        let mut selected_outpoints = Vec::new();
        for outpoint_str in &input_outpoints {
            let parts: Vec<&str> = outpoint_str.split(':').collect();
            if parts.len() != 2 {
                return Err(AccountError::Transaction(format!(
                    "Invalid outpoint format '{outpoint_str}', expected 'txid:vout'"
                )));
            }

            let txid = parts[0].parse::<bitcoin::Txid>().map_err(|e| {
                AccountError::Transaction(format!("Invalid txid in '{outpoint_str}': {e}"))
            })?;
            let vout = parts[1].parse::<u32>().map_err(|e| {
                AccountError::Transaction(format!("Invalid vout in '{outpoint_str}': {e}"))
            })?;

            selected_outpoints.push(OutPoint { txid, vout });
        }

        let all_coins = inner.account.coins();

        let mut available_utxos: Vec<(OutPoint, OwnedOutput)> = Vec::new();
        for outpoint in selected_outpoints {
            match all_coins.get(&outpoint) {
                Some(entry) if entry.is_spendable() => {
                    available_utxos.push((outpoint, entry.owned_output().clone()));
                }
                Some(_) => {
                    return Err(AccountError::Transaction(format!(
                        "Coin {}:{} is not spendable",
                        outpoint.txid, outpoint.vout
                    )));
                }
                None => {
                    return Err(AccountError::Transaction(format!(
                        "Coin {}:{} not found in wallet",
                        outpoint.txid, outpoint.vout
                    )));
                }
            }
        }

        if available_utxos.is_empty() {
            return Err(AccountError::Transaction(
                "No valid spendable coins selected".to_string(),
            ));
        }

        if has_max {
            if !recipients.is_empty() {
                return Err(AccountError::Transaction(
                    "max cannot be combined with other outputs".to_string(),
                ));
            }
            inner
                .client
                .create_drain_transaction(
                    available_utxos,
                    max_addr.unwrap(),
                    fee_rate,
                    inner.account.network(),
                )
                .map_err(|e| AccountError::Transaction(e.to_string()))
        } else {
            use bwk_sp::spdk_core::Recipient;
            let recipients: Vec<Recipient> = recipients
                .into_iter()
                .map(|(address, amount)| Recipient { address, amount })
                .collect();

            inner
                .client
                .create_new_transaction(
                    available_utxos,
                    recipients,
                    fee_rate,
                    inner.account.network(),
                )
                .map_err(|e| AccountError::Transaction(e.to_string()))
        }
    }

    /// Simulate a transaction without actually creating it.
    pub fn simulate_transaction(&self, tx_template: TransactionTemplate) -> TransactionSimulation {
        use bitcoin::Amount;
        use bwk_sp::spdk_core::FeeRate;

        let err_sim = |error: String| TransactionSimulation {
            is_valid: false,
            fee: 0,
            weight: 0,
            input_total: 0,
            output_total: 0,
            input_count: 0,
            error,
            selected_outpoints: Vec::new(),
        };

        let Some(inner) = &self.inner else {
            return err_sim("Account not initialized".to_string());
        };

        let mut recipients = Vec::new();
        let mut has_max = false;
        let mut max_addr = None;

        for output in tx_template.outputs {
            let addr = match RecipientAddress::try_from(output.address.clone()) {
                Ok(a) => a,
                Err(e) => return err_sim(format!("Invalid address '{}': {}", output.address, e)),
            };

            if output.max {
                if has_max {
                    return err_sim("Only one output can have max=true".to_string());
                }
                has_max = true;
                max_addr = Some(addr);
            } else {
                recipients.push((addr, Amount::from_sat(output.amount)));
            }
        }

        let fee_rate = FeeRate::from_sat_per_vb(tx_template.fee_rate.max(1.0) as f32);
        let fees = if tx_template.fee > 0 {
            Fees::Sats(tx_template.fee)
        } else {
            Fees::MilliSatsVb((tx_template.fee_rate.max(1.0) * 1000.0) as u64)
        };

        let unsigned_tx_result = if !tx_template.input_outpoints.is_empty() {
            Self::create_transaction_with_manual_selection(
                inner,
                tx_template.input_outpoints.clone(),
                recipients,
                max_addr,
                has_max,
                fee_rate,
            )
        } else if has_max {
            if !recipients.is_empty() {
                return err_sim("max cannot be combined with other outputs".to_string());
            }
            inner
                .account
                .create_drain_transaction(max_addr.unwrap(), fees.clone())
        } else {
            inner.account.create_transaction(recipients, fees)
        };

        match unsigned_tx_result {
            Ok(unsigned_tx) => {
                let input_total: u64 = unsigned_tx
                    .selected_utxos
                    .iter()
                    .map(|(_, owned)| owned.amount.to_sat())
                    .sum();
                let output_total: u64 = unsigned_tx
                    .recipients
                    .iter()
                    .map(|r| r.amount.to_sat())
                    .sum();
                let fee = input_total.saturating_sub(output_total);
                let input_count = unsigned_tx.selected_utxos.len() as u64;
                let weight = if let Some(ref tx) = unsigned_tx.unsigned_tx {
                    tx.weight().to_wu()
                } else {
                    let vbytes = (fee as f64 / tx_template.fee_rate).ceil() as u64;
                    vbytes * 4
                };

                let selected_outpoints: Vec<String> = unsigned_tx
                    .selected_utxos
                    .iter()
                    .map(|(outpoint, _)| outpoint.to_string())
                    .collect();

                log::debug!(
                    "simulate_transaction: selected {} outpoints: {:?}",
                    selected_outpoints.len(),
                    selected_outpoints
                );

                TransactionSimulation {
                    is_valid: true,
                    fee,
                    weight,
                    input_total,
                    output_total,
                    input_count,
                    error: String::new(),
                    selected_outpoints,
                }
            }
            Err(e) => err_sim(e.to_string()),
        }
    }

    /// Prepare a transaction for signing.
    pub fn prepare_transaction(&self, tx_template: TransactionTemplate) -> Box<PsbtResult> {
        use bitcoin::Amount;
        use bwk_sp::spdk_core::FeeRate;

        let err_psbt = |e: String| Box::new(PsbtResult { inner: Err(e) });

        let Some(inner) = &self.inner else {
            return err_psbt("Account not initialized".to_string());
        };

        let mut recipients = Vec::new();
        let mut has_max = false;
        let mut max_addr = None;

        for output in tx_template.outputs {
            let addr = match RecipientAddress::try_from(output.address.clone()) {
                Ok(a) => a,
                Err(e) => {
                    return err_psbt(format!("Invalid address '{}': {}", output.address, e));
                }
            };

            if output.max {
                if has_max {
                    return err_psbt("Only one output can have max=true".to_string());
                }
                has_max = true;
                max_addr = Some(addr);
            } else {
                recipients.push((addr, Amount::from_sat(output.amount)));
            }
        }

        let fee_rate = FeeRate::from_sat_per_vb(tx_template.fee_rate.max(1.0) as f32);
        let fees = if tx_template.fee > 0 {
            Fees::Sats(tx_template.fee)
        } else {
            Fees::MilliSatsVb((tx_template.fee_rate.max(1.0) * 1000.0) as u64)
        };

        let unsigned_tx_result = if !tx_template.input_outpoints.is_empty() {
            Self::create_transaction_with_manual_selection(
                inner,
                tx_template.input_outpoints.clone(),
                recipients,
                max_addr,
                has_max,
                fee_rate,
            )
        } else if has_max {
            if !recipients.is_empty() {
                return err_psbt("max cannot be combined with other outputs".to_string());
            }
            inner
                .account
                .create_drain_transaction(max_addr.unwrap(), fees.clone())
        } else {
            inner.account.create_transaction(recipients, fees)
        };

        let finalized_result = unsigned_tx_result
            .and_then(|unsigned_tx| inner.account.finalize_transaction(unsigned_tx));

        Box::new(PsbtResult {
            inner: finalized_result.map_err(|e| e.to_string()),
        })
    }

    /// Sign a prepared transaction.
    pub fn sign_transaction(&self, psbt_result: &PsbtResult) -> TxResult {
        use bitcoin::consensus::encode::serialize_hex;

        let Some(inner) = &self.inner else {
            return TxResult {
                is_ok: false,
                error: "Account not initialized".to_string(),
                value: String::new(),
            };
        };

        let unsigned_tx = match &psbt_result.inner {
            Ok(tx) => tx.clone(),
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Cannot sign invalid transaction: {e}"),
                    value: String::new(),
                }
            }
        };

        match inner.account.sign_transaction(unsigned_tx) {
            Ok(signed_tx) => TxResult {
                is_ok: true,
                error: String::new(),
                value: serialize_hex(&signed_tx),
            },
            Err(e) => TxResult {
                is_ok: false,
                error: format!("Signing failed: {e}"),
                value: String::new(),
            },
        }
    }

    /// Broadcast a signed transaction to the network.
    pub fn broadcast_transaction(&self, signed_tx_hex: String) -> TxResult {
        use bitcoin::consensus::encode::deserialize_hex;
        use bitcoin::Transaction;

        let Some(inner) = &self.inner else {
            return TxResult {
                is_ok: false,
                error: "Account not initialized".to_string(),
                value: String::new(),
            };
        };

        let tx: Transaction = match deserialize_hex(&signed_tx_hex) {
            Ok(tx) => tx,
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Invalid transaction hex: {e}"),
                    value: String::new(),
                }
            }
        };

        match inner.account.broadcast(&tx) {
            Ok(txid) => TxResult {
                is_ok: true,
                error: String::new(),
                value: txid.to_string(),
            },
            Err(e) => TxResult {
                is_ok: false,
                error: format!("Broadcast failed: {e}"),
                value: String::new(),
            },
        }
    }

    /// Sign and broadcast a transaction in one step.
    pub fn sign_and_broadcast(&self, psbt_result: &PsbtResult) -> TxResult {
        let Some(inner) = &self.inner else {
            return TxResult {
                is_ok: false,
                error: "Account not initialized".to_string(),
                value: String::new(),
            };
        };

        let unsigned_tx = match &psbt_result.inner {
            Ok(tx) => tx.clone(),
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Cannot sign invalid transaction: {e}"),
                    value: String::new(),
                }
            }
        };

        match inner.account.sign_and_broadcast(unsigned_tx) {
            Ok(txid) => TxResult {
                is_ok: true,
                error: String::new(),
                value: txid.to_string(),
            },
            Err(e) => TxResult {
                is_ok: false,
                error: format!("Sign and broadcast failed: {e}"),
                value: String::new(),
            },
        }
    }
}

/// Convert bwk-sp::Notification to Silent Notification.
fn convert_notification(sp_notif: SpNotification) -> Notification {
    match sp_notif {
        SpNotification::StartingScan => Notification {
            flag: NotificationFlag::StartingScan,
            payload: String::new(),
        },
        SpNotification::ScanStarted { start, end } => Notification {
            flag: NotificationFlag::ScanStarted,
            payload: format!("{start},{end}"),
        },
        SpNotification::FailStartScanning { message } => Notification {
            flag: NotificationFlag::FailStartScanning,
            payload: message,
        },
        SpNotification::FailScan { message } => Notification {
            flag: NotificationFlag::FailScan,
            payload: message,
        },
        SpNotification::StoppingScan => Notification {
            flag: NotificationFlag::StoppingScan,
            payload: String::new(),
        },
        SpNotification::ScanStopped => Notification {
            flag: NotificationFlag::ScanStopped,
            payload: String::new(),
        },
        SpNotification::ScanProgress { current, end } => Notification {
            flag: NotificationFlag::ScanProgress,
            payload: format!("{current},{end}"),
        },
        SpNotification::ScanCompleted => Notification {
            flag: NotificationFlag::ScanCompleted,
            payload: String::new(),
        },
        SpNotification::NewOutput(outpoint) => Notification {
            flag: NotificationFlag::NewOutput,
            payload: outpoint.to_string(),
        },
        SpNotification::OutputSpent(outpoint) => Notification {
            flag: NotificationFlag::OutputSpent,
            payload: outpoint.to_string(),
        },
        SpNotification::WaitingForBlocks { tip_height } => Notification {
            flag: NotificationFlag::WaitingForBlocks,
            payload: tip_height.to_string(),
        },
        SpNotification::NewBlocksDetected {
            from_height,
            to_height,
        } => Notification {
            flag: NotificationFlag::NewBlocksDetected,
            payload: format!("{from_height},{to_height}"),
        },
    }
}

// CXX FFI functions

/// Create a new account.
pub fn new_account(account_name: String) -> Box<Account> {
    let config = match crate::config::Config::from_file(account_name) {
        Ok(c) => c,
        Err(e) => {
            let error = format!("failed to load config: {e}");
            log::error!("{error}");
            return Box::new(Account {
                inner: None,
                error,
            });
        }
    };
    match Account::new_inner(config) {
        Ok(inner) => Box::new(Account {
            inner: Some(inner),
            error: String::new(),
        }),
        Err(e) => {
            let error = format!("Failed to create account: {e}");
            log::error!("{error}");
            Box::new(Account {
                inner: None,
                error,
            })
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
                flag: NotificationFlag::ScanStopped,
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
                flag: NotificationFlag::FailScan,
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
///
/// This type wraps unsigned Silent Payment transactions for Phase 5 (Coin Selection, Signing & Broadcast).
/// It will be used by methods like `prepare_transaction()` to return unsigned transactions that can be
/// signed and broadcast. The type is defined now to establish the CXX bridge interface, but will be
/// actively used once transaction signing and broadcasting are implemented in Phase 5.
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

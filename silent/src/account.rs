//! Account module for Silent.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::net::SocketAddr;
use std::sync::mpsc;
use std::time::Duration;

use bwk_sp::spdk_core::RecipientAddress;
use bwk_sp::{Account as SpAccount, AccountError, Notification as SpNotification, ScanMode, SpRecipientAddress};

use crate::config::Config;
use crate::ffi::{
    CoinState, Notification, NotificationFlag, RustCoin, RustTx, TransactionSimulation,
    TransactionTemplate, TxResult,
};

/// Valid account state (created successfully).
struct AccountInner {
    account: SpAccount,
    receiver: Option<mpsc::Receiver<SpNotification>>,
    network: bitcoin::Network,
    p2p_node: String,
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
        let network = match config.network {
            crate::ffi::Network::Regtest => bitcoin::Network::Regtest,
            crate::ffi::Network::Signet => bitcoin::Network::Signet,
            crate::ffi::Network::Testnet => bitcoin::Network::Testnet,
            crate::ffi::Network::Bitcoin => bitcoin::Network::Bitcoin,
            _ => unreachable!("Invalid network variant"),
        };

        let sp_config = config.to_sp_config();
        let mut account = SpAccount::new(sp_config)?;
        let receiver = account.receiver();

        Ok(AccountInner {
            account,
            receiver,
            network,
            p2p_node: config.p2p_node.clone(),
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

    /// Take ownership of the notification receiver for use in a dedicated thread.
    /// Can only be called once; subsequent calls will panic.
    pub fn take_receiver(&mut self) -> Box<NotificationReceiver> {
        let inner = self.inner.as_mut().expect("Account not initialized");
        let receiver = inner
            .receiver
            .take()
            .expect("Notification receiver already taken");
        Box::new(NotificationReceiver { receiver })
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

    /// Build a configured TxBuilder from a TransactionTemplate.
    fn build_tx_builder(
        inner: &AccountInner,
        tx_template: &TransactionTemplate,
    ) -> Result<bwk_tx::TxBuilder, AccountError> {
        let network = inner.network;
        let mut has_max = false;
        let mut max_addr = None;
        let mut output_total: u64 = 0;
        let mut recipients: Vec<SpRecipientAddress> = Vec::new();

        // Parse outputs
        for output in &tx_template.outputs {
            let addr = RecipientAddress::try_from(output.address.clone()).map_err(|e| {
                AccountError::Transaction(format!("Invalid address '{}': {}", output.address, e))
            })?;

            if output.max {
                if has_max {
                    return Err(AccountError::Transaction(
                        "Only one output can have max=true".to_string(),
                    ));
                }
                has_max = true;
                max_addr = Some(addr);
            } else {
                output_total += output.amount;
                recipients.push(SpRecipientAddress::new(addr, output.amount, network));
            }
        }

        // Get builder (pre-configured with coin_source, sp_provider, change)
        let feerate_msats_vb = (tx_template.fee_rate.max(1.0) * 1000.0) as u64;
        let mut builder = inner.account.tx_builder();
        if tx_template.fee > 0 {
            builder = builder.fee(tx_template.fee);
        } else {
            builder = builder.feerate(feerate_msats_vb);
        }

        // Add outputs
        for recip in recipients {
            builder.add_output(recip);
        }
        if has_max {
            let addr = max_addr.unwrap();
            let mut recip = SpRecipientAddress::new(addr, 0, network);
            recip.amount = bwk_tx::Amount::Max(None);
            builder.add_output(recip);
        }

        // Handle inputs
        if !tx_template.input_outpoints.is_empty() {
            // Manual coin selection
            let all_coins = inner.account.coins();
            for outpoint_str in &tx_template.input_outpoints {
                let outpoint = parse_outpoint(outpoint_str)?;
                let entry = all_coins.get(&outpoint).ok_or_else(|| {
                    AccountError::Transaction(format!(
                        "Coin {outpoint_str} not found in wallet"
                    ))
                })?;
                if !entry.is_spendable() {
                    return Err(AccountError::Transaction(format!(
                        "Coin {outpoint_str} is not spendable"
                    )));
                }
                builder.add_input(sp_coin_entry_to_coin(outpoint, entry));
            }
        } else if has_max {
            // Drain: add all spendable coins
            builder.drain_inputs();
        } else {
            // Auto coin selection
            let coins = builder.select_coins(output_total, feerate_msats_vb);
            if coins.is_empty() {
                return Err(AccountError::Transaction(
                    "Insufficient funds".to_string(),
                ));
            }
            for coin in coins {
                builder.add_input(coin);
            }
        }

        Ok(builder)
    }

    /// Simulate a transaction without actually creating it.
    pub fn simulate_transaction(&self, tx_template: TransactionTemplate) -> TransactionSimulation {
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

        let builder = match Self::build_tx_builder(inner, &tx_template) {
            Ok(b) => b,
            Err(e) => return err_sim(e.to_string()),
        };

        let result = builder.simulate();

        if let Some(ref error) = result.error {
            return err_sim(format!("{error:?}"));
        }

        let input_total: u64 = result
            .tx_template
            .inputs
            .iter()
            .map(|c| c.txout.value.to_sat())
            .sum();
        let fee = result.fees.map(|f| f.to_sat()).unwrap_or(0);
        let output_total = input_total.saturating_sub(fee);
        let input_count = result.tx_template.inputs.len() as u64;

        // Estimate weight from fee and fee_rate
        let weight = if tx_template.fee_rate > 0.0 {
            let vbytes = (fee as f64 / tx_template.fee_rate).ceil() as u64;
            vbytes * 4
        } else {
            0
        };

        let selected_outpoints: Vec<String> = result
            .tx_template
            .inputs
            .iter()
            .map(|c| c.outpoint.to_string())
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

    /// Prepare a transaction for signing.
    pub fn prepare_transaction(&self, tx_template: TransactionTemplate) -> Box<PsbtResult> {
        let err_psbt = |e: String| Box::new(PsbtResult { inner: Err(e) });

        let Some(inner) = &self.inner else {
            return err_psbt("Account not initialized".to_string());
        };

        let mut builder = match Self::build_tx_builder(inner, &tx_template) {
            Ok(b) => b,
            Err(e) => return err_psbt(e.to_string()),
        };

        match builder.generate() {
            Ok(psbt) => Box::new(PsbtResult { inner: Ok(psbt) }),
            Err(e) => err_psbt(format!("{e:?}")),
        }
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

        let mut psbt = match &psbt_result.inner {
            Ok(psbt) => psbt.clone(),
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Cannot sign invalid transaction: {e}"),
                    value: String::new(),
                }
            }
        };

        match inner.account.sign_and_finalize(&mut psbt) {
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

    /// Broadcast a signed transaction to the network via P2P.
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

        let txid = tx.compute_txid();

        if let Err(e) = broadcast_via_p2p(&inner.p2p_node, inner.network, tx) {
            return TxResult {
                is_ok: false,
                error: format!("Broadcast failed: {e}"),
                value: String::new(),
            };
        }

        TxResult {
            is_ok: true,
            error: String::new(),
            value: txid.to_string(),
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

        let mut psbt = match &psbt_result.inner {
            Ok(psbt) => psbt.clone(),
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Cannot sign invalid transaction: {e}"),
                    value: String::new(),
                }
            }
        };

        let signed_tx = match inner.account.sign_and_finalize(&mut psbt) {
            Ok(tx) => tx,
            Err(e) => {
                return TxResult {
                    is_ok: false,
                    error: format!("Signing failed: {e}"),
                    value: String::new(),
                }
            }
        };

        let txid = signed_tx.compute_txid();

        if let Err(e) = broadcast_via_p2p(&inner.p2p_node, inner.network, signed_tx) {
            return TxResult {
                is_ok: false,
                error: format!("Broadcast failed: {e}"),
                value: String::new(),
            };
        }

        TxResult {
            is_ok: true,
            error: String::new(),
            value: txid.to_string(),
        }
    }
}

/// Broadcast a transaction via the P2P network.
fn broadcast_via_p2p(
    p2p_node: &str,
    network: bitcoin::Network,
    tx: bitcoin::Transaction,
) -> Result<(), String> {
    if p2p_node.is_empty() {
        return Err("P2P node address not configured".to_string());
    }

    let addr: SocketAddr = p2p_node
        .parse()
        .map_err(|e| format!("Invalid P2P node address '{p2p_node}': {e}"))?;

    let mut client = bwk_p2p::Client::new(addr, network)
        .timeout(Duration::from_secs(30))
        .connect()
        .map_err(|e| format!("P2P connection failed: {e}"))?;

    let result = client.broadcast_tx(tx).map_err(|e| format!("P2P broadcast failed: {e}"));

    // Give the peer time to receive the tx before closing the connection.
    // broadcast_tx() only writes to the TCP stream; stop() would close it immediately.
    std::thread::sleep(Duration::from_millis(500));

    client.stop();
    result
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

/// Notification receiver that owns the mpsc channel.
/// Designed to be used from a dedicated thread with blocking recv().
pub struct NotificationReceiver {
    receiver: mpsc::Receiver<SpNotification>,
}

impl NotificationReceiver {
    /// Blocking receive — blocks until a notification arrives.
    /// Returns Poll::err when the channel disconnects (sender dropped).
    pub fn recv(&self) -> Box<Poll> {
        match self.receiver.recv() {
            Ok(notif) => {
                let notification = convert_notification(notif);
                Box::new(Poll::ok(notification))
            }
            Err(_) => Box::new(Poll::err("Notification channel disconnected")),
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
pub struct PsbtResult {
    inner: Result<bitcoin::Psbt, String>,
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
            Ok(psbt) => psbt.unsigned_tx.compute_txid().to_string(),
            Err(_) => String::new(),
        }
    }
}

/// Parse an outpoint string ("txid:vout") into a bitcoin::OutPoint.
fn parse_outpoint(s: &str) -> Result<bitcoin::OutPoint, AccountError> {
    let parts: Vec<&str> = s.split(':').collect();
    if parts.len() != 2 {
        return Err(AccountError::Transaction(format!(
            "Invalid outpoint format '{s}', expected 'txid:vout'"
        )));
    }
    let txid = parts[0]
        .parse()
        .map_err(|e| AccountError::Transaction(format!("Invalid txid in '{s}': {e}")))?;
    let vout = parts[1]
        .parse()
        .map_err(|e| AccountError::Transaction(format!("Invalid vout in '{s}': {e}")))?;
    Ok(bitcoin::OutPoint { txid, vout })
}

/// Convert an SpCoinEntry to a bwk_tx::Coin for use with TxBuilder.
fn sp_coin_entry_to_coin(
    outpoint: bitcoin::OutPoint,
    entry: &bwk_sp::SpCoinEntry,
) -> bwk_tx::Coin {
    const TR_KEYSPEND_SATISFACTION_WEIGHT: u64 = 66;
    bwk_tx::Coin {
        txout: bitcoin::TxOut {
            value: entry.amount(),
            script_pubkey: entry.script().clone(),
        },
        outpoint,
        height: Some(entry.height() as u64),
        sequence: bitcoin::Sequence::ENABLE_RBF_NO_LOCKTIME,
        status: bwk_tx::CoinStatus::Confirmed,
        label: None,
        satisfaction_size: TR_KEYSPEND_SATISFACTION_WEIGHT,
        spend_info: bwk_tx::CoinSpendInfo::Sp {
            derivation: bitcoin::bip32::DerivationPath::default(),
            tweak: *entry.tweak(),
        },
    }
}

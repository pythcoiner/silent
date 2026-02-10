//! Account module for Silent.
//!
//! Wraps bwk-sp::Account with CXX-compatible interface for C++ bindings.

use std::sync::mpsc;

use bwk_sp::spdk_core::{
    self, bip39, RecipientAddress, SilentPaymentUnsignedTransaction, SpClient,
};
use bwk_sp::{Account as SpAccount, AccountError, Notification as SpNotification, ScanMode};

use crate::config::Config;
use crate::ffi::{
    CoinState, Notification, NotificationFlag, RustCoin, RustTx, TransactionSimulation,
    TransactionTemplate,
};

/// Account wrapping bwk-sp::Account.
pub struct Account {
    /// Inner bwk-sp Account
    inner: SpAccount,
    /// Notification receiver
    receiver: Option<mpsc::Receiver<SpNotification>>,
    /// SP client for manual transaction creation
    client: SpClient,
}

impl Account {
    /// Create a new Account from config.
    pub fn new(config: Config) -> Result<Self, AccountError> {
        let sp_config = config.to_sp_config();
        let mut inner = SpAccount::new(sp_config)?;
        let receiver = inner.receiver();

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

        Ok(Account {
            inner,
            receiver,
            client,
        })
    }

    /// Start the scanner in continuous mode.
    pub fn start_scanner(&mut self) -> Result<(), String> {
        self.inner
            .start_scan(ScanMode::Continuous)
            .map_err(|e| format!("scanner error: {e}"))
    }

    /// Stop the scanner.
    pub fn stop_scanner(&mut self) {
        self.inner.stop_scan()
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

    /// Create transaction with manual coin selection.
    ///
    /// Filters wallet UTXOs to only use the specified outpoints, then creates
    /// a transaction using the SpClient directly.
    fn create_transaction_with_manual_selection(
        &self,
        input_outpoints: Vec<String>,
        recipients: Vec<(RecipientAddress, bitcoin::Amount)>,
        max_addr: Option<RecipientAddress>,
        has_max: bool,
        fee_rate: spdk_core::FeeRate,
    ) -> Result<SilentPaymentUnsignedTransaction, AccountError> {
        use bitcoin::OutPoint;
        use bwk_sp::spdk_core::OwnedOutput;

        // Parse the input outpoints from "txid:vout" format
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

        // Get all coins from the wallet
        let all_coins = self.inner.coins();

        // Filter to only the specified outpoints
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

        // Create transaction using the SpClient with filtered UTXOs
        if has_max {
            if !recipients.is_empty() {
                return Err(AccountError::Transaction(
                    "max cannot be combined with other outputs".to_string(),
                ));
            }
            self.client
                .create_drain_transaction(
                    available_utxos,
                    max_addr.unwrap(),
                    fee_rate,
                    self.inner.network(),
                )
                .map_err(|e| AccountError::Transaction(e.to_string()))
        } else {
            use bwk_sp::spdk_core::Recipient;
            let recipients: Vec<Recipient> = recipients
                .into_iter()
                .map(|(address, amount)| Recipient { address, amount })
                .collect();

            self.client
                .create_new_transaction(available_utxos, recipients, fee_rate, self.inner.network())
                .map_err(|e| AccountError::Transaction(e.to_string()))
        }
    }

    /// Simulate a transaction without actually creating it.
    pub fn simulate_transaction(&self, tx_template: TransactionTemplate) -> TransactionSimulation {
        use bitcoin::Amount;
        use bwk_sp::spdk_core::FeeRate;

        // Parse outputs and check for max flag
        let mut recipients = Vec::new();
        let mut has_max = false;
        let mut max_addr = None;

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

            if output.max {
                if has_max {
                    return TransactionSimulation {
                        is_valid: false,
                        fee: 0,
                        weight: 0,
                        input_total: 0,
                        output_total: 0,
                        input_count: 0,
                        error: String::from("Only one output can have max=true"),
                    };
                }
                has_max = true;
                max_addr = Some(addr);
            } else {
                recipients.push((addr, Amount::from_sat(output.amount)));
            }
        }

        // Convert fee rate (ensure it's at least 1.0 sat/vb)
        let fee_rate = FeeRate::from_sat_per_vb(tx_template.fee_rate.max(1.0) as f32);

        // Create the unsigned transaction (with manual or automatic coin selection)
        let unsigned_tx_result = if !tx_template.input_outpoints.is_empty() {
            // Manual coin selection: use specified outpoints
            self.create_transaction_with_manual_selection(
                tx_template.input_outpoints,
                recipients,
                max_addr,
                has_max,
                fee_rate,
            )
        } else {
            // Automatic coin selection: use inner Account methods
            if has_max {
                if !recipients.is_empty() {
                    return TransactionSimulation {
                        is_valid: false,
                        fee: 0,
                        weight: 0,
                        input_total: 0,
                        output_total: 0,
                        input_count: 0,
                        error: String::from("max cannot be combined with other outputs"),
                    };
                }
                self.inner
                    .create_drain_transaction(max_addr.unwrap(), fee_rate)
            } else {
                self.inner.create_transaction(recipients, fee_rate)
            }
        };

        // Extract simulation data
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
            Err(e) => TransactionSimulation {
                is_valid: false,
                fee: 0,
                weight: 0,
                input_total: 0,
                output_total: 0,
                input_count: 0,
                error: e.to_string(),
            },
        }
    }

    /// Prepare a transaction for signing.
    ///
    /// This method:
    /// 1. Creates an unsigned transaction (with manual or automatic coin selection)
    /// 2. Finalizes it (generates recipient public keys, builds outputs)
    /// 3. Returns a PsbtResult that can be signed and broadcast
    pub fn prepare_transaction(&self, tx_template: TransactionTemplate) -> Box<PsbtResult> {
        use bitcoin::Amount;
        use bwk_sp::spdk_core::FeeRate;

        // Parse outputs and check for max flag (same logic as simulate_transaction)
        let mut recipients = Vec::new();
        let mut has_max = false;
        let mut max_addr = None;

        for output in tx_template.outputs {
            let addr = match RecipientAddress::try_from(output.address.clone()) {
                Ok(a) => a,
                Err(e) => {
                    return Box::new(PsbtResult {
                        inner: Err(format!("Invalid address '{}': {}", output.address, e)),
                    });
                }
            };

            if output.max {
                if has_max {
                    return Box::new(PsbtResult {
                        inner: Err(String::from("Only one output can have max=true")),
                    });
                }
                has_max = true;
                max_addr = Some(addr);
            } else {
                recipients.push((addr, Amount::from_sat(output.amount)));
            }
        }

        // Convert fee rate
        let fee_rate = FeeRate::from_sat_per_vb(tx_template.fee_rate.max(1.0) as f32);

        // Create the unsigned transaction
        let unsigned_tx_result = if !tx_template.input_outpoints.is_empty() {
            // Manual coin selection
            self.create_transaction_with_manual_selection(
                tx_template.input_outpoints,
                recipients,
                max_addr,
                has_max,
                fee_rate,
            )
        } else {
            // Automatic coin selection
            if has_max {
                if !recipients.is_empty() {
                    return Box::new(PsbtResult {
                        inner: Err(String::from("max cannot be combined with other outputs")),
                    });
                }
                self.inner
                    .create_drain_transaction(max_addr.unwrap(), fee_rate)
            } else {
                self.inner.create_transaction(recipients, fee_rate)
            }
        };

        // Finalize the transaction
        let finalized_result =
            unsigned_tx_result.and_then(|unsigned_tx| self.inner.finalize_transaction(unsigned_tx));

        // Return PsbtResult
        Box::new(PsbtResult {
            inner: finalized_result.map_err(|e| e.to_string()),
        })
    }

    /// Sign a prepared transaction.
    ///
    /// Takes a PsbtResult containing a finalized unsigned transaction,
    /// signs it using the account's spend key, and returns the signed
    /// transaction as a hex string.
    pub fn sign_transaction(&self, psbt_result: &PsbtResult) -> Result<String, String> {
        use bitcoin::consensus::encode::serialize_hex;

        // Extract the unsigned transaction from PsbtResult
        let unsigned_tx = match &psbt_result.inner {
            Ok(tx) => tx.clone(),
            Err(e) => return Err(format!("Cannot sign invalid transaction: {e}")),
        };

        // Sign the transaction using the inner account
        let signed_tx = self
            .inner
            .sign_transaction(unsigned_tx)
            .map_err(|e| format!("Signing failed: {e}"))?;

        // Serialize to hex
        let tx_hex = serialize_hex(&signed_tx);

        Ok(tx_hex)
    }

    /// Broadcast a signed transaction to the network.
    ///
    /// Takes a hex-encoded signed transaction and broadcasts it via the
    /// configured broadcast URL. Returns the txid on success.
    pub fn broadcast_transaction(&self, signed_tx_hex: String) -> Result<String, String> {
        use bitcoin::consensus::encode::deserialize_hex;
        use bitcoin::Transaction;

        // Deserialize the hex transaction
        let tx: Transaction =
            deserialize_hex(&signed_tx_hex).map_err(|e| format!("Invalid transaction hex: {e}"))?;

        // Broadcast using the inner account
        let txid = self
            .inner
            .broadcast(&tx)
            .map_err(|e| format!("Broadcast failed: {e}"))?;

        Ok(txid.to_string())
    }

    /// Sign and broadcast a transaction in one step.
    ///
    /// Convenience method that combines sign_transaction() and broadcast_transaction().
    pub fn sign_and_broadcast(&self, psbt_result: &PsbtResult) -> Result<String, String> {
        // Extract the unsigned transaction from PsbtResult
        let unsigned_tx = match &psbt_result.inner {
            Ok(tx) => tx.clone(),
            Err(e) => return Err(format!("Cannot sign invalid transaction: {e}")),
        };

        // Sign and broadcast using the inner account's convenience method
        let txid = self
            .inner
            .sign_and_broadcast(unsigned_tx)
            .map_err(|e| format!("Sign and broadcast failed: {e}"))?;

        Ok(txid.to_string())
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
pub fn new_account(account_name: String) -> Result<Box<Account>, String> {
    let config = crate::config::Config::from_file(account_name)
        .map_err(|e| format!("failed to load config: {e}"))?;

    Account::new(config).map(Box::new).map_err(|e| {
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

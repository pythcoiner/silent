//! Silent - Bitcoin Silent Payment wallet backed by BlindBit
//!
//! This library provides C++ bindings for the bwk-sp Silent Payment implementation.

pub mod account;
pub mod config;

#[cxx::bridge]
mod ffi {
    // ===== Shared Enums =====

    /// Bitcoin network types.
    #[derive(Debug, Clone, Copy)]
    pub enum Network {
        Regtest,
        Signet,
        Testnet,
        Bitcoin,
    }

    /// Logging levels.
    #[derive(Debug, Clone, Copy)]
    pub enum LogLevel {
        Off,
        Error,
        Warn,
        Info,
        Debug,
        Trace,
    }

    /// Notification types from the scanner.
    #[derive(Debug, Clone, Copy)]
    pub enum NotificationFlag {
        StartingScan,
        ScanStarted,
        FailStartScanning,
        FailScan,
        StoppingScan,
        ScanStopped,
        ScanProgress,
        ScanCompleted,
        NewOutput,
        OutputSpent,
        WaitingForBlocks,
        NewBlocksDetected,
    }

    // ===== Shared Structs =====

    /// Notification from the account scanner.
    #[derive(Debug, Clone)]
    pub struct Notification {
        pub flag: NotificationFlag,
        pub payload: String,
    }

    /// A coin entry from the wallet.
    #[derive(Debug, Clone)]
    pub struct RustCoin {
        pub outpoint: String,
        pub value: u64,
        pub height: u32,
        pub label: String,
        pub spent: bool,
    }

    /// Summary of spendable coins.
    #[derive(Debug, Clone)]
    pub struct CoinState {
        pub confirmed_count: u64,
        pub confirmed_balance: u64,
        pub unconfirmed_count: u64,
        pub unconfirmed_balance: u64,
    }

    /// A transaction entry from payment history.
    #[derive(Debug, Clone)]
    pub struct RustTx {
        pub txid: String,
        pub direction: String,
        pub amount: u64,
        pub fee: u64,
        pub height: u32,
    }

    /// Transaction output specification.
    #[derive(Debug, Clone)]
    pub struct Output {
        /// Recipient address (SP address, legacy address, or hex data for OP_RETURN)
        pub address: String,
        /// Amount in satoshis (must be 0 for OP_RETURN outputs)
        pub amount: u64,
        /// Optional label for this output
        pub label: String,
        /// If true, send maximum possible amount (minus fees) to this output
        pub max: bool,
    }

    /// Template for creating a transaction.
    #[derive(Debug, Clone)]
    pub struct TransactionTemplate {
        /// List of output specifications
        pub outputs: Vec<Output>,
        /// Fee rate in sat/vbyte (used when fee == 0)
        pub fee_rate: f64,
        /// Absolute fee in satoshis (takes precedence over fee_rate when > 0)
        pub fee: u64,
        /// Optional: specific UTXOs to use (empty = automatic selection)
        pub input_outpoints: Vec<String>,
    }

    /// Result of transaction simulation.
    #[derive(Debug, Clone)]
    pub struct TransactionSimulation {
        /// Whether the transaction can be created with current funds
        pub is_valid: bool,
        /// Estimated fee in satoshis (0 if invalid)
        pub fee: u64,
        /// Estimated weight in weight units (0 if invalid)
        pub weight: u64,
        /// Total input amount in satoshis (0 if invalid)
        pub input_total: u64,
        /// Total output amount in satoshis (0 if invalid)
        pub output_total: u64,
        /// Number of inputs selected (0 if invalid)
        pub input_count: u64,
        /// Error message (empty if valid)
        pub error: String,
        /// Outpoints selected by coin selection (empty if invalid)
        pub selected_outpoints: Vec<String>,
    }

    /// Result of a transaction operation (sign, broadcast).
    #[derive(Debug, Clone)]
    pub struct TxResult {
        /// Whether the operation succeeded.
        pub is_ok: bool,
        /// Error message (empty if is_ok is true).
        pub error: String,
        /// Result value: signed tx hex or txid (empty on error).
        pub value: String,
    }

    /// Result of a connection test.
    #[derive(Debug, Clone)]
    pub struct ConnectionResult {
        /// Whether the connection succeeded.
        pub is_ok: bool,
        /// Error message (empty if is_ok is true).
        pub error: String,
    }

    /// Default regtest infrastructure addresses fetched from minta API.
    #[derive(Debug, Clone)]
    pub struct RegtestDefaults {
        /// Whether the fetch succeeded.
        pub is_ok: bool,
        /// Error message (empty if is_ok is true).
        pub error: String,
        /// BlindBit server URL (with http:// prefix).
        pub blindbit_url: String,
        /// P2P node address (host:port).
        pub p2p_node: String,
        /// Electrum server address (host:port).
        pub electrum_url: String,
    }

    /// Backend server information result.
    #[derive(Debug, Clone)]
    pub struct BackendInfo {
        /// Whether the request succeeded.
        pub is_ok: bool,
        /// Error message (empty if is_ok is true).
        pub error: String,
        /// Normalized URL that successfully connected (with scheme).
        pub url: String,
        /// Network reported by the backend.
        pub network: Network,
        /// Current block height.
        pub height: u32,
        /// Backend supports tweaks-only mode.
        pub tweaks_only: bool,
        /// Backend supports full basic tweaks.
        pub tweaks_full_basic: bool,
        /// Backend supports full tweaks with dust filter.
        pub tweaks_full_with_dust_filter: bool,
        /// Backend supports cut-through with dust filter.
        pub tweaks_cut_through_with_dust_filter: bool,
    }

    // ===== Opaque Rust Types =====

    extern "Rust" {
        /// Configuration for a Silent account.
        type Config;

        /// Account instance.
        type Account;

        /// Poll result for checking notifications.
        type Poll;

        /// Result wrapper for unsigned transactions.
        type PsbtResult;

        /// Notification receiver for blocking recv in a dedicated thread.
        type NotificationReceiver;
    }

    // ===== Utility Functions =====

    extern "Rust" {
        /// Initialize logging with the specified level.
        /// Call this once at application startup.
        fn init_logging(level: LogLevel);

        /// Generate a new 12-word BIP39 mnemonic.
        fn generate_mnemonic() -> String;
        fn notification_to_string(notif: &Notification) -> String;

        /// Query backend server info (network, height, capabilities).
        /// Blocking HTTP call - may take up to 30s on timeout.
        fn get_backend_info(blindbit_url: String) -> BackendInfo;

        /// Validate a BIP39 mnemonic string.
        fn validate_mnemonic(mnemonic: String) -> bool;

        /// Validate a recipient address (SP address, legacy Bitcoin address, or hex data).
        /// Returns empty string if valid, or error message if invalid.
        fn validate_address(address: String) -> String;

        /// Test P2P node connectivity.
        /// Blocking call - attempts to connect and perform version handshake.
        fn test_p2p_node(address: String, network: Network) -> ConnectionResult;

        /// Fetch default regtest infrastructure addresses from minta API.
        /// Blocking HTTP call - should be called from a background thread.
        fn get_regtest_defaults() -> RegtestDefaults;
    }

    // ===== Config Methods =====

    extern "Rust" {
        /// Create a new config.
        fn new_config(
            account_name: String,
            network: Network,
            mnemonic: String,
            blindbit_url: String,
            p2p_node: String,
            dust_limit: u64,
        ) -> Box<Config>;

        /// Load config from file.
        fn config_from_file(account_name: String) -> Box<Config>;

        /// List all existing configs.
        fn list_configs() -> Vec<String>;

        /// Delete an account's data from disk.
        fn delete_config(account_name: String) -> bool;

        /// Save config to file.
        fn to_file(self: &Config);

        /// Get mnemonic.
        fn get_mnemonic(self: &Config) -> String;

        /// Set mnemonic.
        fn set_mnemonic(self: &mut Config, mnemonic: String);

        /// Get network.
        fn get_network(self: &Config) -> Network;

        /// Set network.
        fn set_network(self: &mut Config, network: Network);

        /// Get BlindBit URL.
        fn get_blindbit_url(self: &Config) -> String;

        /// Set BlindBit URL.
        fn set_blindbit_url(self: &mut Config, url: String);

        /// Get P2P node address.
        fn get_p2p_node(self: &Config) -> String;

        /// Set P2P node address.
        fn set_p2p_node(self: &mut Config, node: String);

        /// Get dust limit (0 means not set).
        fn get_dust_limit(self: &Config) -> u64;

        /// Set dust limit (0 to unset).
        fn set_dust_limit(self: &mut Config, limit: u64);
    }

    // ===== Account Methods =====

    extern "Rust" {
        /// Create a new account from an account name (loads config from file).
        /// Check is_ok() before use; get_error() for failure reason.
        fn new_account(account_name: String) -> Box<Account>;

        /// Check if the account was created successfully.
        fn is_ok(self: &Account) -> bool;

        /// Get error message (empty if is_ok).
        fn get_error(self: &Account) -> String;

        /// Start the scanner.
        fn start_scanner(self: &mut Account) -> bool;

        /// Stop the scanner.
        fn stop_scanner(self: &mut Account);

        /// Try to receive a notification (non-blocking).
        fn try_recv(self: &mut Account) -> Box<Poll>;

        /// Take ownership of the notification receiver for use in a dedicated thread.
        fn take_receiver(self: &mut Account) -> Box<NotificationReceiver>;

        /// Get account name.
        fn name(self: &Account) -> String;

        /// Get balance in satoshis.
        fn balance(self: &Account) -> u64;

        /// Get the Silent Payment address.
        fn sp_address(self: &Account) -> String;

        /// Get all coins (spent and unspent).
        fn coins(self: &Account) -> Vec<RustCoin>;

        /// Get spendable coins summary.
        fn spendable_coins(self: &Account) -> CoinState;

        /// Update a coin's label.
        fn update_coin_label(self: &mut Account, outpoint: String, label: String) -> bool;

        /// Get payment history.
        fn payment_history(self: &Account) -> Vec<RustTx>;

        /// Simulate a transaction to check feasibility and estimate fees.
        /// Returns simulation result with fee, weight, and validity info.
        fn simulate_transaction(
            self: &Account,
            tx_template: TransactionTemplate,
        ) -> TransactionSimulation;

        /// Prepare a transaction for signing.
        /// Creates an unsigned transaction from the template and returns a PsbtResult.
        /// The transaction will be finalized and ready to sign.
        fn prepare_transaction(self: &Account, tx_template: TransactionTemplate)
            -> Box<PsbtResult>;

        /// Sign a prepared transaction.
        /// Returns TxResult with signed tx hex in value.
        fn sign_transaction(self: &Account, psbt_result: &PsbtResult) -> TxResult;

        /// Broadcast a signed transaction (hex string) to the network.
        /// Returns TxResult with txid in value.
        fn broadcast_transaction(self: &Account, signed_tx_hex: String) -> TxResult;

        /// Sign and broadcast a transaction in one step.
        /// Returns TxResult with txid in value.
        fn sign_and_broadcast(self: &Account, psbt_result: &PsbtResult) -> TxResult;
    }

    // ===== Poll Methods =====

    extern "Rust" {
        /// Check if poll has a notification.
        fn is_some(self: &Poll) -> bool;

        /// Get the notification (only valid if is_some() == true).
        fn get_notification(self: &Poll) -> Notification;

        /// Get error message (if any).
        fn get_error(self: &Poll) -> String;
    }

    // ===== NotificationReceiver Methods =====

    extern "Rust" {
        /// Blocking receive — waits for the next notification.
        /// Returns Poll with is_some()=false when the channel disconnects.
        fn recv(self: &NotificationReceiver) -> Box<Poll>;
    }

    // ===== PsbtResult Methods =====

    extern "Rust" {
        /// Check if result is valid.
        fn is_ok(self: &PsbtResult) -> bool;

        /// Get error message (only valid if !is_ok()).
        fn get_psbt_error(self: &PsbtResult) -> String;

        /// Get transaction ID preview (only valid if is_ok()).
        fn get_txid_preview(self: &PsbtResult) -> String;
    }
}

/// Initialize logging with the specified level.
pub fn init_logging(level: LogLevel) {
    let filter = match level {
        LogLevel::Off => log::LevelFilter::Off,
        LogLevel::Error => log::LevelFilter::Error,
        LogLevel::Warn => log::LevelFilter::Warn,
        LogLevel::Info => log::LevelFilter::Info,
        LogLevel::Debug => log::LevelFilter::Debug,
        LogLevel::Trace => log::LevelFilter::Trace,
        _ => log::LevelFilter::Info,
    };
    env_logger::Builder::new()
        .filter_level(filter)
        .filter_module("ureq", log::LevelFilter::Info)
        .init();
}

/// Generate a new 12-word BIP39 mnemonic.
pub fn generate_mnemonic() -> String {
    let mut entropy = [0u8; 16]; // 128 bits = 12 words
    getrandom::getrandom(&mut entropy).expect("failed to generate random entropy");
    bwk_sp::spdk_core::bip39::Mnemonic::from_entropy(&entropy)
        .expect("mnemonic generation from 128-bit entropy should not fail")
        .to_string()
}

pub fn notification_to_string(notif: &Notification) -> String {
    format!("{notif:?}")
}

/// Query backend info. Returns a BackendInfo with is_ok=false on error.
pub fn get_backend_info(blindbit_url: String) -> BackendInfo {
    match bwk_sp::backend_info(blindbit_url) {
        Ok((info, url)) => BackendInfo {
            is_ok: true,
            error: String::new(),
            url,
            network: info.network.into(),
            height: info.height.to_consensus_u32(),
            tweaks_only: info.tweaks_only,
            tweaks_full_basic: info.tweaks_full_basic,
            tweaks_full_with_dust_filter: info.tweaks_full_with_dust_filter,
            tweaks_cut_through_with_dust_filter: info.tweaks_cut_through_with_dust_filter,
        },
        Err(e) => BackendInfo {
            is_ok: false,
            error: e.to_string(),
            url: String::new(),
            network: Network::Regtest,
            height: 0,
            tweaks_only: false,
            tweaks_full_basic: false,
            tweaks_full_with_dust_filter: false,
            tweaks_cut_through_with_dust_filter: false,
        },
    }
}

/// Validate a BIP39 mnemonic string.
pub fn validate_mnemonic(mnemonic: String) -> bool {
    bwk_sp::spdk_core::bip39::Mnemonic::parse(&mnemonic).is_ok()
}

/// Validate a recipient address (SP address, legacy Bitcoin address, or hex data).
/// Returns empty string if valid, or error message if invalid.
pub fn validate_address(address: String) -> String {
    if address.is_empty() {
        return String::new();
    }
    match bwk_sp::spdk_core::RecipientAddress::try_from(address) {
        Ok(_) => String::new(),
        Err(e) => e.to_string(),
    }
}

/// Test P2P node connectivity by attempting a version handshake.
pub fn test_p2p_node(address: String, network: Network) -> ffi::ConnectionResult {
    use std::net::SocketAddr;
    use std::time::Duration;

    log::info!("test_p2p_node()");
    let addr: SocketAddr = match address.parse() {
        Ok(a) => a,
        Err(e) => {
            return ffi::ConnectionResult {
                is_ok: false,
                error: format!("Invalid address '{address}': {e}"),
            }
        }
    };

    let btc_network: bitcoin::Network = network.into();

    log::info!("test_p2p_node() create client");
    let mut client = match bwk_p2p::Client::new(addr, btc_network)
        .timeout(Duration::from_secs(1))
        .connect()
    {
        Ok(c) => c,
        Err(e) => {
            return ffi::ConnectionResult {
                is_ok: false,
                error: format!("Connection failed: {e}"),
            }
        }
    };

    log::info!("test_p2p_node() stop client");
    client.stop();

    ffi::ConnectionResult {
        is_ok: true,
        error: String::new(),
    }
}

/// Fetch default regtest infrastructure addresses from minta.pythcoiner.dev.
pub fn get_regtest_defaults() -> RegtestDefaults {
    let url = "http://minta.pythcoiner.dev/api/status";

    let mut response = match ureq::get(url).call() {
        Ok(r) => r,
        Err(e) => {
            log::warn!("failed to fetch regtest defaults: {e}");
            return RegtestDefaults {
                is_ok: false,
                error: format!("HTTP request failed: {e}"),
                blindbit_url: String::new(),
                p2p_node: String::new(),
                electrum_url: String::new(),
            };
        }
    };

    let body: String = match response.body_mut().read_to_string() {
        Ok(s) => s,
        Err(e) => {
            log::warn!("failed to read regtest defaults response: {e}");
            return RegtestDefaults {
                is_ok: false,
                error: format!("Failed to read response: {e}"),
                blindbit_url: String::new(),
                p2p_node: String::new(),
                electrum_url: String::new(),
            };
        }
    };

    let json: serde_json::Value = match serde_json::from_str(&body) {
        Ok(v) => v,
        Err(e) => {
            log::warn!("failed to parse regtest defaults JSON: {e}");
            return RegtestDefaults {
                is_ok: false,
                error: format!("JSON parse failed: {e}"),
                blindbit_url: String::new(),
                p2p_node: String::new(),
                electrum_url: String::new(),
            };
        }
    };

    let blindbit_connect = json["blindbit_connect"].as_str().unwrap_or_default();
    let p2p_connect = json["p2p_connect"].as_str().unwrap_or_default();
    let electrum_connect = json["electrum_connect"].as_str().unwrap_or_default();

    if blindbit_connect.is_empty() || p2p_connect.is_empty() {
        return RegtestDefaults {
            is_ok: false,
            error: "Missing fields in API response".to_string(),
            blindbit_url: String::new(),
            p2p_node: String::new(),
            electrum_url: String::new(),
        };
    }

    RegtestDefaults {
        is_ok: true,
        error: String::new(),
        blindbit_url: format!("http://{blindbit_connect}"),
        p2p_node: p2p_connect.to_string(),
        electrum_url: electrum_connect.to_string(),
    }
}

// Re-export main types
pub use account::{new_account, Account, NotificationReceiver, Poll, PsbtResult};
pub use config::{config_from_file, delete_config, list_configs, new_config, set_datadir, Config};
pub use ffi::{
    BackendInfo, LogLevel, Network, Notification, NotificationFlag, Output, RegtestDefaults,
    TransactionSimulation, TransactionTemplate,
};

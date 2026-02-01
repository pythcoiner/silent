//! Templar - Bitcoin Silent Payment wallet backed by BlindBit
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
        ScanStarted,
        ScanProgress,
        ScanCompleted,
        ScanError,
        NewOutput,
        OutputSpent,
        Stopped,
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

    // ===== Opaque Rust Types =====

    extern "Rust" {
        /// Configuration for a Templar account.
        type Config;

        /// Account instance.
        type Account;

        /// Poll result for checking notifications.
        type Poll;
    }

    // ===== Config Methods =====

    extern "Rust" {
        /// Create a new config.
        fn new_config(
            account_name: String,
            network: Network,
            mnemonic: String,
            blindbit_url: String,
            dust_limit: u64,
        ) -> Box<Config>;

        /// Load config from file.
        fn config_from_file(account_name: String) -> Box<Config>;

        /// List all existing configs.
        fn list_configs() -> Vec<String>;

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

        /// Get dust limit (0 means not set).
        fn get_dust_limit(self: &Config) -> u64;

        /// Set dust limit (0 to unset).
        fn set_dust_limit(self: &mut Config, limit: u64);
    }

    // ===== Account Methods =====

    extern "Rust" {
        /// Create a new account from an account name (loads config from file).
        /// Returns null on error (check logs for details).
        fn new_account(account_name: String) -> Result<Box<Account>>;

        /// Start the scanner.
        fn start_scanner(self: &mut Account) -> Result<()>;

        /// Stop the scanner.
        fn stop_scanner(self: &mut Account);

        /// Try to receive a notification (non-blocking).
        fn try_recv(self: &mut Account) -> Box<Poll>;

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
        fn update_coin_label(self: &mut Account, outpoint: String, label: String) -> Result<()>;

        /// Get payment history.
        fn payment_history(self: &Account) -> Vec<RustTx>;
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
}

// Re-export main types
pub use account::{new_account, Account, Poll};
pub use config::{config_from_file, list_configs, new_config, Config};
pub use ffi::{LogLevel, Network, Notification, NotificationFlag};

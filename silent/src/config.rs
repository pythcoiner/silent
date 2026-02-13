//! Configuration module for Silent accounts.
//!
//! Wraps bwk-sp::Config with CXX-compatible interface for C++ bindings.

use std::fs;
use std::path::PathBuf;
use std::sync::OnceLock;

use bwk_sp::Config as SpConfig;
use bitcoin::Network as BtcNetwork;
use serde::{Deserialize, Serialize};

use crate::ffi::Network;

/// Internal Network type for serialization (since CXX enums don't support Serde).
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
enum NetworkInternal {
    Regtest,
    Signet,
    Testnet,
    Bitcoin,
}

impl From<Network> for NetworkInternal {
    fn from(value: Network) -> Self {
        match value {
            Network::Regtest => NetworkInternal::Regtest,
            Network::Signet => NetworkInternal::Signet,
            Network::Testnet => NetworkInternal::Testnet,
            Network::Bitcoin => NetworkInternal::Bitcoin,
            // SAFETY: Network enum is defined in CXX bridge with exactly 4 variants.
            // CXX guarantees type safety - invalid values cannot be constructed.
            // This wildcard exists only for exhaustiveness checking.
            _ => unreachable!("CXX enum type safety guarantees no other variants exist"),
        }
    }
}

impl From<NetworkInternal> for Network {
    fn from(value: NetworkInternal) -> Self {
        match value {
            NetworkInternal::Regtest => Network::Regtest,
            NetworkInternal::Signet => Network::Signet,
            NetworkInternal::Testnet => Network::Testnet,
            NetworkInternal::Bitcoin => Network::Bitcoin,
        }
    }
}

/// Configuration for a Silent account.
///
/// Wraps bwk-sp::Config with simplified interface for CXX FFI.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    /// Account name
    pub account_name: String,
    /// Bitcoin network (stored as internal type for serde)
    #[serde(with = "network_serde")]
    pub network: Network,
    /// BIP39 mnemonic phrase
    pub mnemonic: String,
    /// Blindbit server URL
    pub blindbit_url: String,
    /// Optional dust limit in satoshis
    pub dust_limit: Option<u64>,
    /// Base directory for account data
    #[serde(skip)]
    pub data_dir: PathBuf,
}

/// Serde serialization helpers for Network.
mod network_serde {
    use super::*;
    use serde::{Deserialize, Deserializer, Serialize, Serializer};

    pub fn serialize<S>(network: &Network, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        NetworkInternal::from(*network).serialize(serializer)
    }

    pub fn deserialize<'de, D>(deserializer: D) -> Result<Network, D::Error>
    where
        D: Deserializer<'de>,
    {
        NetworkInternal::deserialize(deserializer).map(Network::from)
    }
}

impl Config {
    /// Create a new Config.
    pub fn new(
        account_name: String,
        network: Network,
        mnemonic: String,
        blindbit_url: String,
        dust_limit: Option<u64>,
    ) -> Self {
        let data_dir = datadir();
        Self {
            account_name,
            network,
            mnemonic,
            blindbit_url,
            dust_limit,
            data_dir,
        }
    }

    /// Load Config from file.
    pub fn from_file(account_name: String) -> Result<Self, ConfigError> {
        let path = Self::config_path_for_account(&account_name);
        let content = fs::read_to_string(&path)
            .map_err(|e| ConfigError::Io(format!("failed to read config: {e}")))?;
        let mut config: Config = serde_json::from_str(&content)
            .map_err(|e| ConfigError::Parse(format!("failed to parse config: {e}")))?;
        config.account_name = account_name;
        config.data_dir = datadir();
        Ok(config)
    }

    /// Save Config to file.
    pub fn to_file(&self) {
        let path = self.config_path();
        if let Some(parent) = path.parent() {
            let _ = fs::create_dir_all(parent);
        }

        match serde_json::to_string_pretty(self) {
            Ok(content) => {
                if let Err(e) = fs::write(&path, content) {
                    log::error!("Config::to_file() failed to write: {e}");
                }
            }
            Err(e) => log::error!("Config::to_file() failed to serialize: {e}"),
        }
    }

    /// Get account directory path.
    pub fn account_dir(&self) -> PathBuf {
        self.data_dir.join(&self.account_name)
    }

    /// Get config file path.
    pub fn config_path(&self) -> PathBuf {
        self.account_dir().join("config.json")
    }

    /// Get config file path for a specific account.
    fn config_path_for_account(account_name: &str) -> PathBuf {
        datadir().join(account_name).join("config.json")
    }

    /// Convert to bwk-sp::Config.
    pub fn to_sp_config(&self) -> SpConfig {
        let mut config = SpConfig::new(
            self.account_name.clone(),
            self.network.into(),
            self.mnemonic.clone(),
            self.blindbit_url.clone(),
            self.data_dir.clone(),
        );
        config.set_dust_limit(self.dust_limit);
        config.set_birthday_height(Some(1));
        config.enable_persist(true)
    }

    /// Get mnemonic.
    pub fn get_mnemonic(&self) -> String {
        self.mnemonic.clone()
    }

    /// Set mnemonic.
    pub fn set_mnemonic(&mut self, mnemonic: String) {
        self.mnemonic = mnemonic;
    }

    /// Get network.
    pub fn get_network(&self) -> Network {
        self.network
    }

    /// Set network.
    pub fn set_network(&mut self, network: Network) {
        self.network = network;
    }

    /// Get BlindBit URL.
    pub fn get_blindbit_url(&self) -> String {
        self.blindbit_url.clone()
    }

    /// Set BlindBit URL.
    pub fn set_blindbit_url(&mut self, url: String) {
        self.blindbit_url = url;
    }

    /// Get dust limit.
    pub fn get_dust_limit(&self) -> u64 {
        self.dust_limit.unwrap_or(0)
    }

    /// Set dust limit.
    pub fn set_dust_limit(&mut self, limit: u64) {
        self.dust_limit = if limit > 0 { Some(limit) } else { None };
    }
}

/// Override for the data directory. Once set, all calls to `datadir()` use this path.
static DATADIR_OVERRIDE: OnceLock<PathBuf> = OnceLock::new();

/// Override the data directory. Intended for testing.
/// Must be called before any other config operations. Can only be set once.
pub fn set_datadir(path: PathBuf) {
    DATADIR_OVERRIDE.set(path).expect("datadir already set");
}

/// Returns the base data directory for Silent.
///
/// On Linux: ~/.silent
/// On other systems: {config_dir}/Silent
pub fn datadir() -> PathBuf {
    if let Some(dir) = DATADIR_OVERRIDE.get() {
        maybe_create_dir(dir);
        return dir.clone();
    }
    #[cfg(target_os = "linux")]
    let dir = {
        let mut dir = dirs::home_dir().expect("home directory not found");
        dir.push(".silent");
        dir
    };

    #[cfg(not(target_os = "linux"))]
    let dir = {
        let mut dir = dirs::config_dir().expect("config directory not found");
        dir.push("Silent");
        dir
    };

    maybe_create_dir(&dir);
    dir
}

/// Create directory if it doesn't exist.
fn maybe_create_dir(dir: &PathBuf) {
    if !dir.exists() {
        #[cfg(unix)]
        {
            use std::fs::DirBuilder;
            use std::os::unix::fs::DirBuilderExt;

            let mut builder = DirBuilder::new();
            builder
                .mode(0o700)
                .recursive(true)
                .create(dir)
                .expect("failed to create directory");
        }

        #[cfg(not(unix))]
        fs::create_dir_all(dir).expect("failed to create directory");
    }
}

/// Configuration errors.
#[derive(Debug, thiserror::Error)]
pub enum ConfigError {
    #[error("io error: {0}")]
    Io(String),
    #[error("parse error: {0}")]
    Parse(String),
}

// CXX FFI functions

/// Create a new config.
pub fn new_config(
    account_name: String,
    network: Network,
    mnemonic: String,
    blindbit_url: String,
    dust_limit: u64,
) -> Box<Config> {
    let dust = if dust_limit > 0 {
        Some(dust_limit)
    } else {
        None
    };
    Box::new(Config::new(
        account_name,
        network,
        mnemonic,
        blindbit_url,
        dust,
    ))
}

/// Delete an account's data from disk.
pub fn delete_config(account_name: String) -> bool {
    let data_dir = datadir();
    match SpConfig::delete_account_dir(&data_dir, &account_name) {
        Ok(()) => true,
        Err(e) => {
            log::error!("failed to delete account '{account_name}': {e}");
            false
        }
    }
}

/// Load config from file.
pub fn config_from_file(account_name: String) -> Box<Config> {
    match Config::from_file(account_name.clone()) {
        Ok(config) => Box::new(config),
        Err(_) => {
            // Return a default config if file doesn't exist
            Box::new(Config::new(
                account_name,
                Network::Signet,
                String::new(),
                String::new(),
                None,
            ))
        }
    }
}

/// List all config directories.
pub fn list_configs() -> Vec<String> {
    let path = datadir();
    let mut out = vec![];
    if let Ok(folders) = fs::read_dir(path) {
        for entry in folders.flatten() {
            if let Ok(md) = entry.metadata() {
                if md.is_dir() {
                    let acc_name = entry.file_name().to_string_lossy().to_string();
                    if Config::from_file(acc_name.clone()).is_ok() {
                        out.push(acc_name);
                    }
                }
            }
        }
    }
    out
}

impl From<Network> for BtcNetwork {
    fn from(value: Network) -> Self {
        match value {
            Network::Regtest => BtcNetwork::Regtest,
            Network::Signet => BtcNetwork::Signet,
            Network::Testnet => BtcNetwork::Testnet,
            Network::Bitcoin => BtcNetwork::Bitcoin,
            // SAFETY: Network enum is defined in CXX bridge with exactly 4 variants.
            // CXX guarantees type safety - invalid values cannot be constructed.
            // This wildcard exists only for exhaustiveness checking.
            _ => unreachable!("CXX enum type safety guarantees no other variants exist"),
        }
    }
}

impl From<BtcNetwork> for Network {
    fn from(value: BtcNetwork) -> Self {
        match value {
            BtcNetwork::Regtest => Network::Regtest,
            BtcNetwork::Signet => Network::Signet,
            BtcNetwork::Testnet => Network::Testnet,
            BtcNetwork::Bitcoin => Network::Bitcoin,
            // SAFETY: bitcoin::Network only has 4 variants that we explicitly handle.
            // This wildcard exists only for exhaustiveness checking.
            _ => unreachable!("bitcoin::Network enum only has 4 variants"),
        }
    }
}

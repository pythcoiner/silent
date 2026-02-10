//! End-to-end integration tests for Silent wallet.
//!
//! These tests cover wallet creation, config management, SP address generation,
//! BlindBit connection, send/receive flows, and wallet restore functionality.

use silent::{Account, Config, Network, NotificationFlag};
use std::thread;
use std::time::Duration;

// ===== Test Helpers =====

/// Generate unique test account name using process ID.
fn test_account_name() -> String {
    format!("test_account_{}", std::process::id())
}

/// Create test config with regtest network.
fn create_test_config(account_name: String) -> Config {
    Config::new(
        account_name,
        Network::Regtest,
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about".to_string(),
        "http://localhost:50001".to_string(),
        Some(546), // Standard dust limit
    )
}

/// Cleanup test account directory.
fn cleanup_test_account(account_name: &str) {
    use silent::config::Config;
    let config = Config::from_file(account_name.to_string()).ok();
    if let Some(cfg) = config {
        let _ = std::fs::remove_dir_all(cfg.account_dir());
    }
}

// ===== Wallet Creation and Config Tests =====

#[test]
fn test_wallet_creation_from_mnemonic() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());

    // Verify config fields
    assert_eq!(config.account_name, account_name);
    assert_eq!(config.get_network(), Network::Regtest);
    assert!(!config.get_mnemonic().is_empty());

    // Create account from config
    let account_result = Account::new(config.clone());
    assert!(account_result.is_ok(), "Account creation should succeed");

    let account = account_result.unwrap();
    assert_eq!(account.name(), account_name);
    assert_eq!(account.balance(), 0); // New wallet should have zero balance

    cleanup_test_account(&account_name);
}

#[test]
fn test_config_save_load_roundtrip() {
    let account_name = test_account_name();
    let original_config = create_test_config(account_name.clone());

    // Save config to file
    original_config.to_file();

    // Verify file exists
    assert!(original_config.config_path().exists());

    // Load config from file
    let loaded_config = Config::from_file(account_name.clone())
        .expect("Config should load successfully");

    // Verify all fields match
    assert_eq!(loaded_config.account_name, original_config.account_name);
    assert_eq!(loaded_config.get_network(), original_config.get_network());
    assert_eq!(loaded_config.get_mnemonic(), original_config.get_mnemonic());
    assert_eq!(loaded_config.get_blindbit_url(), original_config.get_blindbit_url());
    assert_eq!(loaded_config.get_dust_limit(), original_config.get_dust_limit());

    cleanup_test_account(&account_name);
}

// ===== SP Address Generation Tests =====

#[test]
fn test_sp_address_generation() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    let sp_addr = account.sp_address();

    // Verify SP address format
    // SP addresses start with "sp1" (mainnet), "tsp1" (testnet/signet), or "sprt1" (regtest)
    assert!(!sp_addr.is_empty(), "SP address should not be empty");
    assert!(sp_addr.starts_with("sp") || sp_addr.starts_with("tsp"),
            "SP address should start with sp prefix");

    // Verify address length (SP addresses are typically 118 characters)
    assert!(sp_addr.len() >= 100, "SP address should be at least 100 characters, got {}", sp_addr.len());

    // Verify address is deterministic (same mnemonic -> same address)
    let account_name2 = format!("{}_2", account_name);
    let config2 = create_test_config(account_name2.clone());
    let account2 = Account::new(config2).expect("Account creation should succeed");
    assert_eq!(account2.sp_address(), sp_addr, "Same mnemonic should produce same address");

    cleanup_test_account(&account_name);
    cleanup_test_account(&account_name2);
}

// ===== BlindBit Connection Tests =====

#[test]
#[ignore] // Requires local BlindBit server on port 50001
fn test_blindbit_connection_regtest() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner
    let start_result = account.start_scanner();
    assert!(start_result.is_ok(), "Scanner should start successfully: {:?}", start_result.err());

    // Wait for ScanStarted notification
    thread::sleep(Duration::from_millis(500));
    let poll = account.try_recv();

    if poll.is_some() {
        let notif = poll.get_notification();
        // Should receive ScanStarted or ScanProgress
        assert!(matches!(notif.flag, NotificationFlag::ScanStarted | NotificationFlag::ScanProgress),
                "Expected ScanStarted or ScanProgress, got {:?}", notif.flag);
    }

    // Stop scanner
    account.stop_scanner();

    // Wait for Stopped notification
    thread::sleep(Duration::from_millis(500));
    let poll = account.try_recv();
    if poll.is_some() {
        let notif = poll.get_notification();
        assert_eq!(notif.flag, NotificationFlag::Stopped, "Expected Stopped notification");
    }

    cleanup_test_account(&account_name);
}

// ===== Send/Receive SP Flow Tests =====

#[test]
#[ignore] // Requires local regtest Bitcoin node and BlindBit server
fn test_send_receive_sp_flow() {
    // This test requires:
    // 1. Local regtest Bitcoin node running
    // 2. Local BlindBit server connected to regtest node
    // 3. Initial funding of the test wallet

    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner
    account.start_scanner().expect("Scanner should start");

    // Wait for initial sync (adjust timeout as needed)
    thread::sleep(Duration::from_secs(5));

    // Check initial balance (should be 0 for new wallet)
    let initial_balance = account.balance();
    println!("Initial balance: {} sats", initial_balance);

    // NOTE: Manual step required - fund this address before running test:
    println!("Fund this address: {}", account.sp_address());
    println!("Waiting 30 seconds for funding transaction...");
    thread::sleep(Duration::from_secs(30));

    // Check for NewOutput notification
    let mut received_output = false;
    for _ in 0..10 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if matches!(notif.flag, NotificationFlag::NewOutput) {
                println!("Received output: {}", notif.payload);
                received_output = true;
            }
        }
        thread::sleep(Duration::from_millis(500));
    }

    if received_output {
        use silent::{TransactionTemplate, Output};

        // Verify balance increased
        let new_balance = account.balance();
        assert!(new_balance > initial_balance, "Balance should increase after receiving funds");

        // Create send transaction (send half back to same address for testing)
        let send_amount = new_balance / 2;
        let tx_template = TransactionTemplate {
            outputs: vec![Output {
                address: account.sp_address(),
                amount: send_amount,
                label: String::from("test send"),
                max: false,
            }],
            fee_rate: 1.0,
            input_outpoints: vec![],
        };

        // Simulate transaction
        let simulation = account.simulate_transaction(tx_template.clone());
        assert!(simulation.is_valid, "Transaction simulation should succeed: {}", simulation.error);
        println!("Simulation: fee={} sats, weight={} WU", simulation.fee, simulation.weight);

        // Prepare transaction
        let psbt = account.prepare_transaction(tx_template);
        assert!(psbt.is_ok(), "Transaction preparation should succeed: {}", psbt.get_psbt_error());

        // Sign and broadcast
        let txid_result = account.sign_and_broadcast(&psbt);
        assert!(txid_result.is_ok(), "Broadcast should succeed: {:?}", txid_result.err());

        let txid = txid_result.unwrap();
        println!("Broadcast txid: {}", txid);
        assert_eq!(txid.len(), 64, "Txid should be 64 hex characters");
    } else {
        println!("WARNING: No funding received, skipping send test");
    }

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

// ===== Wallet Restore Tests =====

#[test]
fn test_wallet_restore_from_mnemonic() {
    let account_name = test_account_name();
    let mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

    // Create first wallet
    let config1 = Config::new(
        account_name.clone(),
        Network::Regtest,
        mnemonic.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );
    let account1 = Account::new(config1).expect("First account creation should succeed");
    let addr1 = account1.sp_address();

    // Delete wallet data
    cleanup_test_account(&account_name);

    // Restore wallet with same mnemonic
    let config2 = Config::new(
        account_name.clone(),
        Network::Regtest,
        mnemonic.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );
    let account2 = Account::new(config2).expect("Restored account creation should succeed");
    let addr2 = account2.sp_address();

    // Verify same address (proves key derivation is deterministic)
    assert_eq!(addr1, addr2, "Restored wallet should have same SP address");

    cleanup_test_account(&account_name);
}

// ===== Signet Testing =====

#[test]
#[ignore] // Manual test requiring signet BlindBit server
fn test_signet_connection() {
    let account_name = test_account_name();
    let config = Config::new(
        account_name.clone(),
        Network::Signet,
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about".to_string(),
        "https://blindbit-signet.example.com".to_string(), // Replace with actual signet BlindBit URL
        Some(546),
    );

    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner
    let start_result = account.start_scanner();
    assert!(start_result.is_ok(), "Signet scanner should start: {:?}", start_result.err());

    // Wait for sync to complete
    println!("Syncing with signet BlindBit server...");
    let mut scan_completed = false;
    for _ in 0..60 { // Wait up to 30 seconds
        thread::sleep(Duration::from_millis(500));
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            println!("Notification: {:?} - {}", notif.flag, notif.payload);
            if matches!(notif.flag, NotificationFlag::ScanCompleted) {
                scan_completed = true;
                break;
            }
        }
    }

    assert!(scan_completed, "Scan should complete within timeout");

    // Verify address generation works on signet
    let addr = account.sp_address();
    assert!(!addr.is_empty(), "Signet address should not be empty");
    assert!(addr.len() >= 100, "Signet address should be at least 100 characters");

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

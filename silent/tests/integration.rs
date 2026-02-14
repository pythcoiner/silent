//! End-to-end integration tests for Silent wallet.
//!
//! These tests cover wallet creation, config management, SP address generation,
//! BlindBit connection, send/receive flows, and wallet restore functionality.

mod common;

use bwk_utils::test as bwk_test;
use silent::{Account, Config, Network, NotificationFlag};
use std::thread;
use std::time::Duration;

use common::{
    cleanup_test_account, create_test_account, fund_sp_wallet, setup_blindbitd, test_account_name,
    wait_for_scan_complete, wait_for_sync_and_index, TEST_MNEMONIC,
};

// ===== Test Helpers =====

/// Create test config with regtest network.
fn create_test_config(account_name: String) -> Config {
    Config::new(
        account_name,
        Network::Regtest,
        TEST_MNEMONIC.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    )
}

// ===== Wallet Creation and Config Tests =====

#[test]
fn test_wallet_creation_from_mnemonic() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());

    assert_eq!(config.account_name, account_name);
    assert_eq!(config.get_network(), Network::Regtest);
    assert!(!config.get_mnemonic().is_empty());

    let account_result = Account::new(config.clone());
    assert!(account_result.is_ok(), "Account creation should succeed");

    let account = account_result.unwrap();
    assert_eq!(account.name(), account_name);
    assert_eq!(account.balance(), 0);

    cleanup_test_account(&account_name);
}

#[test]
fn test_config_save_load_roundtrip() {
    let account_name = test_account_name();
    let original_config = create_test_config(account_name.clone());

    original_config.to_file();
    assert!(original_config.config_path().exists());

    let loaded_config =
        Config::from_file(account_name.clone()).expect("Config should load successfully");

    assert_eq!(loaded_config.account_name, original_config.account_name);
    assert_eq!(loaded_config.get_network(), original_config.get_network());
    assert_eq!(loaded_config.get_mnemonic(), original_config.get_mnemonic());
    assert_eq!(
        loaded_config.get_blindbit_url(),
        original_config.get_blindbit_url()
    );
    assert_eq!(
        loaded_config.get_dust_limit(),
        original_config.get_dust_limit()
    );

    cleanup_test_account(&account_name);
}

// ===== SP Address Generation Tests =====

#[test]
fn test_sp_address_generation() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    let sp_addr = account.sp_address();

    assert!(!sp_addr.is_empty(), "SP address should not be empty");
    assert!(
        sp_addr.starts_with("sp") || sp_addr.starts_with("tsp"),
        "SP address should start with sp prefix"
    );
    assert!(
        sp_addr.len() >= 100,
        "SP address should be at least 100 characters, got {}",
        sp_addr.len()
    );

    // Verify address is deterministic
    let account_name2 = format!("{}_2", account_name);
    let config2 = create_test_config(account_name2.clone());
    let account2 = Account::new(config2).expect("Account creation should succeed");
    assert_eq!(
        account2.sp_address(),
        sp_addr,
        "Same mnemonic should produce same address"
    );

    cleanup_test_account(&account_name);
    cleanup_test_account(&account_name2);
}

// ===== BlindBit Connection Tests =====

#[test]
fn test_blindbit_connection_regtest() {
    let (bbd, mut bitcoind_node) = setup_blindbitd();
    let bitcoind = &mut bitcoind_node.client;
    let url = bbd.url();

    // Generate blocks for coinbase maturity
    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&url, 101);

    let account_name = test_account_name();
    let mut account = create_test_account(&account_name, &url);

    // Start scanner
    assert!(account.start_scanner(), "Scanner should start successfully");

    // Wait for scanner to connect and start scanning
    let got_started = wait_for_scan_complete(&mut account, 30);
    assert!(
        got_started,
        "Scanner should complete scan or reach tip within 30s"
    );

    // Stop scanner
    account.stop_scanner();

    // Wait for ScanStopped
    let mut got_stopped = false;
    for _ in 0..50 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if notif.flag == NotificationFlag::ScanStopped {
                got_stopped = true;
                break;
            }
        }
        thread::sleep(Duration::from_millis(100));
    }
    assert!(got_stopped, "Should receive ScanStopped notification");

    cleanup_test_account(&account_name);
    drop(bbd);
}

// ===== Send/Receive SP Flow Tests =====

#[test]
fn test_send_receive_sp_flow() {
    let (bbd, mut bitcoind_node) = setup_blindbitd();
    let bitcoind = &mut bitcoind_node.client;
    let url = bbd.url();

    // Generate blocks for coinbase maturity
    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&url, 101);

    let account_name = test_account_name();
    let mut account = create_test_account(&account_name, &url);

    // Start scanner and wait for initial sync
    assert!(account.start_scanner(), "Scanner should start");
    assert!(
        wait_for_scan_complete(&mut account, 30),
        "Initial scan should complete"
    );

    let initial_balance = account.balance();
    assert_eq!(initial_balance, 0, "New wallet should have zero balance");

    // Fund the wallet with an SP transaction
    fund_sp_wallet(bitcoind, &url, TEST_MNEMONIC, 0.1);

    // Wait for the scanner to detect the new output
    let mut received_output = false;
    for _ in 0..300 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if notif.flag == NotificationFlag::NewOutput {
                received_output = true;
                break;
            }
        }
        thread::sleep(Duration::from_millis(100));
    }
    assert!(received_output, "Should receive NewOutput notification");

    // Verify balance increased
    let new_balance = account.balance();
    assert!(
        new_balance > 0,
        "Balance should increase after receiving funds, got {}",
        new_balance
    );

    // Create send transaction (send back to same address)
    use silent::{Output, TransactionTemplate};

    let send_amount = new_balance / 2;
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: account.sp_address(),
            amount: send_amount,
            label: String::from("test send"),
            max: false,
        }],
        fee_rate: 1.0,
        fee: 0,
        input_outpoints: vec![],
    };

    // Simulate transaction
    let simulation = account.simulate_transaction(tx_template.clone());
    assert!(
        simulation.is_valid,
        "Transaction simulation should succeed: {}",
        simulation.error
    );
    assert!(simulation.fee > 0, "Fee should be non-zero");

    // Prepare transaction
    let psbt = account.prepare_transaction(tx_template);
    assert!(
        psbt.is_ok(),
        "Transaction preparation should succeed: {}",
        psbt.get_psbt_error()
    );

    // Note: sign_and_broadcast requires a broadcast URL (Esplora-style HTTP endpoint)
    // which isn't available on regtest. Transaction preparation is sufficient to verify
    // the full SP transaction lifecycle up to signing.

    account.stop_scanner();
    cleanup_test_account(&account_name);
    drop(bbd);
}

// ===== Wallet Restore Tests =====

#[test]
fn test_wallet_restore_from_mnemonic() {
    let account_name = test_account_name();

    let config1 = Config::new(
        account_name.clone(),
        Network::Regtest,
        TEST_MNEMONIC.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );
    let account1 = Account::new(config1).expect("First account creation should succeed");
    let addr1 = account1.sp_address();

    cleanup_test_account(&account_name);

    let config2 = Config::new(
        account_name.clone(),
        Network::Regtest,
        TEST_MNEMONIC.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );
    let account2 = Account::new(config2).expect("Restored account creation should succeed");
    let addr2 = account2.sp_address();

    assert_eq!(
        addr1, addr2,
        "Restored wallet should have same SP address"
    );

    cleanup_test_account(&account_name);
}

// ===== Signet Testing =====

#[test]
#[ignore] // Requires real signet BlindBit server (BlindbitD is regtest only)
fn test_signet_connection() {
    let account_name = test_account_name();
    let config = Config::new(
        account_name.clone(),
        Network::Signet,
        TEST_MNEMONIC.to_string(),
        "https://blindbit-signet.example.com".to_string(),
        Some(546),
    );

    let mut account = Account::new(config).expect("Account creation should succeed");

    assert!(
        account.start_scanner(),
        "Signet scanner should start"
    );

    let scan_completed = wait_for_scan_complete(&mut account, 30);
    assert!(scan_completed, "Scan should complete within timeout");

    let addr = account.sp_address();
    assert!(!addr.is_empty(), "Signet address should not be empty");
    assert!(
        addr.len() >= 100,
        "Signet address should be at least 100 characters"
    );

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

// ===== Connection Loss Tests =====

#[test]
fn test_connection_loss_retry() {
    let (mut bbd, mut bitcoind_node) = setup_blindbitd();
    let bitcoind = &mut bitcoind_node.client;
    let url = bbd.url();

    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&url, 101);

    let account_name = test_account_name();
    let mut account = create_test_account(&account_name, &url);

    assert!(
        account.start_scanner(),
        "Initial start should succeed"
    );
    assert!(
        wait_for_scan_complete(&mut account, 30),
        "Initial scan should complete"
    );

    // Kill BlindbitD to simulate connection loss
    bbd.kill().expect("kill blindbitd");

    // Wait for FailScan notification
    let mut error_count = 0;
    for _ in 0..100 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if matches!(
                notif.flag,
                NotificationFlag::FailScan | NotificationFlag::FailStartScanning
            ) {
                error_count += 1;
                break;
            }
        }
        thread::sleep(Duration::from_millis(200));
    }

    assert!(
        error_count > 0,
        "Should receive at least one FailScan after connection loss"
    );

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

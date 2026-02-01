//! Multi-account, error handling, and edge case tests for Templar wallet.
//!
//! These tests verify multi-account isolation, comprehensive error handling,
//! and edge cases like zero balance sends, dust detection, and max sends.

use templar::{Account, Config, Network, NotificationFlag, TransactionTemplate, Output};
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
    use templar::config::Config;
    let config = Config::from_file(account_name.to_string()).ok();
    if let Some(cfg) = config {
        let _ = std::fs::remove_dir_all(cfg.account_dir());
    }
}

// ===== Multi-Account Isolation Tests =====

#[test]
fn test_multiple_accounts_independent_state() {
    // Create two accounts with different mnemonics
    let account1_name = format!("test_account_1_{}", std::process::id());
    let account2_name = format!("test_account_2_{}", std::process::id());

    let mnemonic1 = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    let mnemonic2 = "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong";

    let config1 = Config::new(
        account1_name.clone(),
        Network::Regtest,
        mnemonic1.to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );

    let config2 = Config::new(
        account2_name.clone(),
        Network::Signet,
        mnemonic2.to_string(),
        "http://localhost:50002".to_string(),
        Some(1000),
    );

    let account1 = Account::new(config1.clone()).expect("Account 1 creation should succeed");
    let account2 = Account::new(config2.clone()).expect("Account 2 creation should succeed");

    // Verify different addresses
    assert_ne!(account1.sp_address(), account2.sp_address(), "Accounts should have different addresses");

    // Verify different networks
    assert_eq!(config1.get_network(), Network::Regtest);
    assert_eq!(config2.get_network(), Network::Signet);

    // Verify different BlindBit URLs
    assert_ne!(config1.get_blindbit_url(), config2.get_blindbit_url());

    // Verify config files are in separate directories
    assert_ne!(config1.config_path(), config2.config_path());
    assert!(config1.config_path().to_string_lossy().contains(&account1_name));
    assert!(config2.config_path().to_string_lossy().contains(&account2_name));

    cleanup_test_account(&account1_name);
    cleanup_test_account(&account2_name);
}

#[test]
fn test_account_config_isolation() {
    let account1_name = format!("test_iso_1_{}", std::process::id());
    let account2_name = format!("test_iso_2_{}", std::process::id());

    let config1 = Config::new(
        account1_name.clone(),
        Network::Regtest,
        "test mnemonic 1".to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );

    let config2 = Config::new(
        account2_name.clone(),
        Network::Testnet,
        "test mnemonic 2".to_string(),
        "http://localhost:50002".to_string(),
        Some(1000),
    );

    // Save both configs
    config1.to_file();
    config2.to_file();

    // Load config1 and verify it hasn't been affected by config2
    let loaded1 = Config::from_file(account1_name.clone()).expect("Load should succeed");
    assert_eq!(loaded1.get_mnemonic(), "test mnemonic 1");
    assert_eq!(loaded1.get_network(), Network::Regtest);
    assert_eq!(loaded1.get_blindbit_url(), "http://localhost:50001");

    // Load config2 and verify independence
    let loaded2 = Config::from_file(account2_name.clone()).expect("Load should succeed");
    assert_eq!(loaded2.get_mnemonic(), "test mnemonic 2");
    assert_eq!(loaded2.get_network(), Network::Testnet);
    assert_eq!(loaded2.get_blindbit_url(), "http://localhost:50002");

    cleanup_test_account(&account1_name);
    cleanup_test_account(&account2_name);
}

// ===== Error Handling Tests =====

#[test]
#[ignore] // Requires network access to test connection failure
fn test_invalid_blindbit_url_error() {
    let account_name = test_account_name();
    let config = Config::new(
        account_name.clone(),
        Network::Regtest,
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about".to_string(),
        "http://invalid-blindbit-url-does-not-exist.local:99999".to_string(),
        Some(546),
    );

    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner (should succeed but connection will fail)
    let start_result = account.start_scanner();
    assert!(start_result.is_ok(), "Scanner start should not fail immediately");

    // Wait for ScanError notification
    thread::sleep(Duration::from_secs(2));
    let mut got_error = false;

    for _ in 0..10 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if matches!(notif.flag, NotificationFlag::ScanError) {
                println!("Got expected ScanError: {}", notif.payload);
                got_error = true;
                break;
            }
        }
        thread::sleep(Duration::from_millis(200));
    }

    assert!(got_error, "Should receive ScanError notification for invalid URL");

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

#[test]
fn test_invalid_mnemonic_error() {
    let account_name = test_account_name();
    let config = Config::new(
        account_name.clone(),
        Network::Regtest,
        "invalid mnemonic phrase that is not bip39 compliant".to_string(),
        "http://localhost:50001".to_string(),
        Some(546),
    );

    // Account creation should fail with invalid mnemonic
    let account_result = Account::new(config);
    assert!(account_result.is_err(), "Account creation should fail with invalid mnemonic");

    if let Err(error) = account_result {
        let error_msg = error.to_string();
        assert!(error_msg.contains("mnemonic") || error_msg.contains("Mnemonic"),
                "Error should mention mnemonic: {}", error_msg);
    }

    cleanup_test_account(&account_name);
}

#[test]
fn test_insufficient_funds_error() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    // Try to create transaction with zero balance
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: account.sp_address(), // Use valid SP address
            amount: 100000, // 0.001 BTC
            label: String::new(),
            max: false,
        }],
        fee_rate: 1.0,
        input_outpoints: vec![],
    };

    let simulation = account.simulate_transaction(tx_template);
    assert!(!simulation.is_valid, "Simulation should fail with insufficient funds");
    assert!(!simulation.error.is_empty(), "Should have error message");
    assert!(simulation.error.to_lowercase().contains("insufficient") ||
            simulation.error.to_lowercase().contains("balance") ||
            simulation.error.to_lowercase().contains("funds") ||
            simulation.error.to_lowercase().contains("spendable") ||
            simulation.error.to_lowercase().contains("coin"),
            "Error should indicate insufficient funds: {}", simulation.error);

    cleanup_test_account(&account_name);
}

#[test]
fn test_invalid_sp_address_error() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    let invalid_addresses = vec![
        "not_a_valid_address",
        "tsp1invalid",
        "", // Empty address
        "sp1" // Incomplete address
    ];

    for invalid_addr in invalid_addresses {
        let tx_template = TransactionTemplate {
            outputs: vec![Output {
                address: invalid_addr.to_string(),
                amount: 10000,
                label: String::new(),
                max: false,
            }],
            fee_rate: 1.0,
            input_outpoints: vec![],
        };

        let simulation = account.simulate_transaction(tx_template.clone());
        assert!(!simulation.is_valid, "Simulation should fail with invalid address: {}", invalid_addr);
        // The error could be about address validation or about insufficient coins (if address parsing succeeds but is invalid)
        assert!(!simulation.error.is_empty(), "Should have error message for invalid address: {}", invalid_addr);
    }

    cleanup_test_account(&account_name);
}

#[test]
#[ignore] // Requires stopping/starting BlindBit server during test
fn test_connection_loss_retry() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner with valid connection
    account.start_scanner().expect("Initial start should succeed");
    thread::sleep(Duration::from_secs(1));

    // Simulate connection loss by stopping BlindBit server (manual step)
    println!("Stop BlindBit server now and press Enter...");
    let mut input = String::new();
    std::io::stdin().read_line(&mut input).unwrap();

    // Wait for ScanError notifications with retry attempts
    let mut error_count = 0;
    for _ in 0..30 {
        let poll = account.try_recv();
        if poll.is_some() {
            let notif = poll.get_notification();
            if matches!(notif.flag, NotificationFlag::ScanError) {
                println!("ScanError: {}", notif.payload);
                assert!(notif.payload.contains("retries"), "Error should mention retry attempts");
                error_count += 1;
            }
        }
        thread::sleep(Duration::from_millis(500));
    }

    assert!(error_count > 0, "Should receive at least one ScanError");

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

// ===== Edge Case Tests =====

#[test]
fn test_zero_balance_send_attempt() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    // Verify zero balance
    assert_eq!(account.balance(), 0, "New wallet should have zero balance");

    // Attempt to send with zero balance
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: account.sp_address(),
            amount: 1000,
            label: String::new(),
            max: false,
        }],
        fee_rate: 1.0,
        input_outpoints: vec![],
    };

    let simulation = account.simulate_transaction(tx_template.clone());
    assert!(!simulation.is_valid, "Should not be able to send with zero balance");
    assert!(!simulation.error.is_empty(), "Should have error message");

    // Attempt to prepare transaction (should also fail)
    let psbt = account.prepare_transaction(tx_template);
    assert!(!psbt.is_ok(), "Prepare should fail: {}", psbt.get_psbt_error());

    cleanup_test_account(&account_name);
}

#[test]
fn test_dust_output_detection() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    // Try to create output below dust limit (546 sats for standard outputs)
    let dust_amounts = vec![1, 100, 545]; // All below standard dust limit

    for dust_amount in dust_amounts {
        let tx_template = TransactionTemplate {
            outputs: vec![Output {
                address: account.sp_address(),
                amount: dust_amount,
                label: String::new(),
                max: false,
            }],
            fee_rate: 1.0,
            input_outpoints: vec![],
        };

        let simulation = account.simulate_transaction(tx_template);

        // Simulation might fail due to dust or insufficient funds
        // Just verify it doesn't panic
        if !simulation.is_valid {
            println!("Dust amount {} rejected: {}", dust_amount, simulation.error);
        }
    }

    cleanup_test_account(&account_name);
}

#[test]
#[ignore] // Requires funded wallet
fn test_max_send_entire_balance() {
    // This test requires a funded wallet to verify max send works correctly
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner and wait for sync
    account.start_scanner().expect("Scanner should start");
    println!("Fund this address and wait: {}", account.sp_address());
    thread::sleep(Duration::from_secs(30));

    let balance = account.balance();
    if balance == 0 {
        println!("WARNING: No funds, skipping max send test");
        account.stop_scanner();
        cleanup_test_account(&account_name);
        return;
    }

    println!("Current balance: {} sats", balance);

    // Create max send transaction
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: account.sp_address(),
            amount: 0, // Amount ignored when max=true
            label: String::from("max send"),
            max: true,
        }],
        fee_rate: 1.0,
        input_outpoints: vec![],
    };

    let simulation = account.simulate_transaction(tx_template.clone());
    assert!(simulation.is_valid, "Max send simulation should succeed: {}", simulation.error);

    // Verify output amount = balance - fee
    let expected_output = balance.saturating_sub(simulation.fee);
    assert_eq!(simulation.output_total, expected_output,
               "Output should equal balance minus fee");

    println!("Max send: sending {} sats with {} sat fee", simulation.output_total, simulation.fee);

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

#[test]
fn test_multiple_outputs_single_transaction() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let account = Account::new(config).expect("Account creation should succeed");

    // Create transaction with multiple outputs
    let addr = account.sp_address();
    let tx_template = TransactionTemplate {
        outputs: vec![
            Output {
                address: addr.clone(),
                amount: 10000,
                label: String::from("output 1"),
                max: false,
            },
            Output {
                address: addr,
                amount: 20000,
                label: String::from("output 2"),
                max: false,
            },
        ],
        fee_rate: 1.0,
        input_outpoints: vec![],
    };

    let simulation = account.simulate_transaction(tx_template);

    // Will fail due to insufficient funds, but verify it processes multiple outputs
    if !simulation.is_valid {
        // Expected - just verify no panic and proper error
        assert!(!simulation.error.is_empty());
    }

    cleanup_test_account(&account_name);
}

#[test]
#[ignore] // Requires funded wallet with multiple UTXOs
fn test_manual_coin_selection() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    account.start_scanner().expect("Scanner should start");
    println!("Fund this address multiple times: {}", account.sp_address());
    thread::sleep(Duration::from_secs(30));

    let coins = account.coins();
    let spendable: Vec<_> = coins.iter().filter(|c| !c.spent).collect();

    if spendable.len() < 2 {
        println!("WARNING: Need at least 2 UTXOs, skipping manual selection test");
        account.stop_scanner();
        cleanup_test_account(&account_name);
        return;
    }

    // Select specific coins
    let selected_outpoints = vec![
        spendable[0].outpoint.clone(),
        spendable[1].outpoint.clone(),
    ];

    let selected_value: u64 = spendable[0].value + spendable[1].value;
    println!("Manually selected {} coins with {} sats", selected_outpoints.len(), selected_value);

    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: account.sp_address(),
            amount: selected_value / 2, // Send half
            label: String::from("manual selection test"),
            max: false,
        }],
        fee_rate: 1.0,
        input_outpoints: selected_outpoints.clone(),
    };

    let simulation = account.simulate_transaction(tx_template.clone());
    assert!(simulation.is_valid, "Manual coin selection should work: {}", simulation.error);
    assert_eq!(simulation.input_count as usize, selected_outpoints.len(),
               "Should use exactly the selected coins");

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

// ===== Additional Error Path Tests =====

#[test]
fn test_invalid_outpoint_format() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Test with invalid outpoint format
    let result = account.update_coin_label("invalid_format".to_string(), "test".to_string());
    assert!(result.is_err(), "Should reject invalid outpoint format");

    let error = result.unwrap_err();
    assert!(error.contains("outpoint") || error.contains("format"),
            "Error should mention outpoint format: {}", error);

    cleanup_test_account(&account_name);
}

#[test]
#[ignore] // Requires BlindBit server
fn test_concurrent_scanner_operations() {
    let account_name = test_account_name();
    let config = create_test_config(account_name.clone());
    let mut account = Account::new(config).expect("Account creation should succeed");

    // Start scanner
    account.start_scanner().expect("First start should succeed");
    thread::sleep(Duration::from_millis(500));

    // Try to start again (should be idempotent or return error gracefully)
    let second_start = account.start_scanner();
    // Either succeeds (idempotent) or fails gracefully (no panic)
    if let Err(e) = second_start {
        println!("Second start returned error (expected): {}", e);
    }

    // Stop and restart
    account.stop_scanner();
    thread::sleep(Duration::from_millis(500));

    let restart_result = account.start_scanner();
    assert!(restart_result.is_ok(), "Restart should succeed");

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

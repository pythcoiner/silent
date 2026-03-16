//! Integration tests for PSBT pre-signing validation via Electrum.
//!
//! Tests validate_before_sign() which checks for output address reuse
//! and already-spent inputs before signing a transaction.

mod common;

use bwk_utils::test as bwk_test;
use silent::{Output, TransactionTemplate};
use std::thread;
use std::time::Duration;

use common::{
    cleanup_test_account, create_test_account_with_electrum, fund_sp_wallet,
    setup_blindbitd_with_electrum, test_account_name, wait_for_scan_complete,
    wait_for_sync_and_index, TEST_MNEMONIC,
};

// ===== PSBT Validation Tests =====

/// Validate a PSBT sending to a fresh (never-used) address.
/// Should pass with is_valid=true, no issues.
#[test]
fn test_validate_before_sign_clean() {
    let account_name = test_account_name();
    let (_bbd, mut bitcoind_node, electrum_url) = setup_blindbitd_with_electrum();
    let bitcoind = &mut bitcoind_node.client;
    let bbd_url = _bbd.url();

    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&bbd_url, 101);

    // Fund and scan
    let mut account =
        create_test_account_with_electrum(&account_name, &bbd_url, &electrum_url);
    account.start_scanner();
    fund_sp_wallet(bitcoind, &bbd_url, TEST_MNEMONIC, 0.1);
    assert!(
        wait_for_scan_complete(&mut account, 60),
        "Scan should complete"
    );

    let balance = account.balance();
    assert!(balance > 0, "Should have balance after funding");

    // Generate a fresh taproot address from the account (never used on-chain)
    let fresh_addr = account.new_taproot_addr();
    assert!(!fresh_addr.is_empty(), "Should generate a taproot address");

    // Build and prepare tx to the fresh address
    let send_amount = balance / 2;
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: fresh_addr,
            amount: send_amount,
            label: String::from("test clean validation"),
            max: false,
        }],
        fee_rate: 1.0,
        fee: 0,
        input_outpoints: vec![],
    };

    let psbt = account.prepare_transaction(tx_template);
    assert!(
        psbt.is_ok(),
        "Prepare should succeed: {}",
        psbt.get_psbt_error()
    );

    // Validate — should be clean
    let validation = account.validate_before_sign(&psbt);
    assert!(
        validation.is_ok,
        "Validation check should succeed: {}",
        validation.error
    );
    assert!(
        validation.is_valid,
        "PSBT should be valid (no issues): {}",
        validation.issues
    );
    assert_eq!(
        validation.reused_output_count, 0,
        "No reused outputs expected"
    );
    assert_eq!(
        validation.spent_input_count, 0,
        "No spent inputs expected"
    );

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

/// Validate a PSBT sending to an address that already has on-chain history.
/// Should detect address reuse (reused_output_count > 0).
#[test]
fn test_validate_before_sign_address_reuse() {
    let account_name = test_account_name();
    let (_bbd, mut bitcoind_node, electrum_url) = setup_blindbitd_with_electrum();
    let bitcoind = &mut bitcoind_node.client;
    let bbd_url = _bbd.url();

    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&bbd_url, 101);

    // Fund and scan
    let mut account =
        create_test_account_with_electrum(&account_name, &bbd_url, &electrum_url);
    account.start_scanner();
    fund_sp_wallet(bitcoind, &bbd_url, TEST_MNEMONIC, 0.1);
    assert!(
        wait_for_scan_complete(&mut account, 60),
        "Scan should complete"
    );

    let balance = account.balance();
    assert!(balance > 0, "Should have balance after funding");

    // Get a bitcoind address and fund it so it has on-chain history
    let reused_addr = bitcoind.new_address().expect("new bitcoind address");
    bwk_test::send(bitcoind, reused_addr.clone(), 0.001)
        .expect("fund reused address");
    bwk_test::generate_blocks(bitcoind, 1);

    let height = bwk_test::get_height(bitcoind);
    wait_for_sync_and_index(&bbd_url, height as u32);
    // Extra wait for electrum indexing
    thread::sleep(Duration::from_secs(3));

    // Build tx sending to that already-used address
    let send_amount = balance / 2;
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: reused_addr.to_string(),
            amount: send_amount,
            label: String::from("test reuse detection"),
            max: false,
        }],
        fee_rate: 1.0,
        fee: 0,
        input_outpoints: vec![],
    };

    let psbt = account.prepare_transaction(tx_template);
    assert!(
        psbt.is_ok(),
        "Prepare should succeed: {}",
        psbt.get_psbt_error()
    );

    // Validate — should detect address reuse
    let validation = account.validate_before_sign(&psbt);
    assert!(
        validation.is_ok,
        "Validation check should succeed: {}",
        validation.error
    );
    assert!(
        !validation.is_valid,
        "PSBT should NOT be valid (address reuse detected)"
    );
    assert!(
        validation.reused_output_count > 0,
        "Should detect reused output address, got: {}",
        validation.reused_output_count
    );

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

/// Validate a PSBT whose inputs have already been spent on-chain.
/// Should detect spent inputs (spent_input_count > 0).
#[test]
fn test_validate_before_sign_spent_input() {
    let account_name = test_account_name();
    let (_bbd, mut bitcoind_node, electrum_url) = setup_blindbitd_with_electrum();
    let bitcoind = &mut bitcoind_node.client;
    let bbd_url = _bbd.url();

    bwk_test::generate_blocks(bitcoind, 101);
    wait_for_sync_and_index(&bbd_url, 101);

    // Fund and scan
    let mut account =
        create_test_account_with_electrum(&account_name, &bbd_url, &electrum_url);
    account.start_scanner();
    fund_sp_wallet(bitcoind, &bbd_url, TEST_MNEMONIC, 0.1);
    assert!(
        wait_for_scan_complete(&mut account, 60),
        "Scan should complete"
    );

    let balance = account.balance();
    assert!(balance > 0, "Should have balance after funding");

    // Prepare a tx spending our coins
    let fresh_addr = account.new_taproot_addr();
    let send_amount = balance / 2;
    let tx_template = TransactionTemplate {
        outputs: vec![Output {
            address: fresh_addr,
            amount: send_amount,
            label: String::from("test spent input"),
            max: false,
        }],
        fee_rate: 1.0,
        fee: 0,
        input_outpoints: vec![],
    };

    let psbt = account.prepare_transaction(tx_template);
    assert!(
        psbt.is_ok(),
        "Prepare should succeed: {}",
        psbt.get_psbt_error()
    );

    // Sign the tx and broadcast via bitcoind RPC
    let signed = account.sign_transaction(&psbt);
    assert!(signed.is_ok, "Sign should succeed: {}", signed.error);

    let tx: bitcoin::Transaction =
        bitcoin::consensus::encode::deserialize_hex(&signed.value).expect("valid tx hex");
    bitcoind
        .send_raw_transaction(&tx)
        .expect("broadcast signed tx");
    bwk_test::generate_blocks(bitcoind, 1);

    let height = bwk_test::get_height(bitcoind);
    wait_for_sync_and_index(&bbd_url, height as u32);
    // Extra wait for electrum indexing
    thread::sleep(Duration::from_secs(3));

    // Now validate the same (stale) PSBT — inputs are already spent
    let validation = account.validate_before_sign(&psbt);
    assert!(
        validation.is_ok,
        "Validation check should succeed: {}",
        validation.error
    );
    assert!(
        !validation.is_valid,
        "PSBT should NOT be valid (inputs already spent)"
    );
    assert!(
        validation.spent_input_count > 0,
        "Should detect spent inputs, got: {}",
        validation.spent_input_count
    );

    account.stop_scanner();
    cleanup_test_account(&account_name);
}

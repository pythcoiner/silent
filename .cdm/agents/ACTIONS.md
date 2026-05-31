# Build and Test Actions

This file defines the commands used to build, test, and verify this project. Fill in the sections below to configure verification behavior.

## Build Commands

Commands to compile/build the project:

### Primary Build

```bash
# Add your build command here
# Example: cargo build --release
# Example: npm run build
# Example: make
```

### Development Build

```bash
# Add your development build command here (if different)
# Example: cargo build
# Example: npm run dev
```

## Lint Commands

Commands to check code quality and style:

### Primary Linter

```bash
# Add your linting command here
# Example: cargo clippy -- -D warnings
# Example: npm run lint
# Example: pylint src/
```

### Additional Linters

```bash
# Add any additional linting commands here (optional)
# Example: cargo fmt -- --check
```

## Test Commands

Commands to run tests:

### Unit Tests

```bash
# Add your unit test command here
# Example: cargo test
# Example: npm test
# Example: pytest tests/
```

### Integration Tests

```bash
# Add your integration test command here (if separate)
# Example: cargo test --test integration
```

### All Tests

```bash
# Add command to run all tests
# Example: cargo test --all
```

## Verification Sequence

The order in which verification steps should run:

1. (First step - e.g., "Build")
2. (Second step - e.g., "Lint")
3. (Third step - e.g., "Test")

## Environment Setup

Any environment variables or setup needed before running commands:

```bash
# Add any required environment setup here
# Example: export RUST_BACKTRACE=1
# Example: source .env
```

## Clean Commands

Commands to clean build artifacts:

```bash
# Add your clean command here
# Example: cargo clean
# Example: npm run clean
# Example: make clean
```

## Notes

Special considerations for running these commands:

- (Note 1 - e.g., "Tests require Docker to be running")
- (Note 2 - e.g., "Build takes approximately 5 minutes")

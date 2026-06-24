# Trading Systems Lab

Two production-style C++ trading systems projects built as a portfolio repo.

## Projects

### 1. Exchange Matching Engine

Implements:

- Limit orders
- Market orders
- Order cancellation
- Price-time priority matching
- Multi-threaded order submission through a lock-free queue
- Custom fixed-size memory pool allocator
- Latency statistics: p50, p99, p99.9
- Binary protocol encode/decode helpers
- Benchmark executable

### 2. End-to-End HFT Trading Platform

Implements:

- UDP market data feed publisher/listener
- Order book maintenance
- Strategy engine based on order-book imbalance
- Risk checks
- Order gateway
- Exchange simulator
- Historical backtesting
- Metrics dashboard data generation

## Quick Start

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
powershell -ExecutionPolicy Bypass -File scripts\run_demos.ps1
```

On Linux/macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Live Dashboard

The `docs/` folder contains a static dashboard. GitHub Actions publishes it to GitHub Pages on every push to `main`.

## Repository Structure

```text
exchange_matching_engine/  Standalone matching engine library, tests, benchmark
hft_trading_platform/      End-to-end simulated HFT workflow
docs/                      Static dashboard for live metrics
scripts/                   Local build/test/demo helpers
.github/workflows/         CI and GitHub Pages deployment
```

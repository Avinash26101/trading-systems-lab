$ErrorActionPreference = "Stop"

if (!(Test-Path build)) {
  .\scripts\build.ps1
}

.\build\exchange_matching_engine\exchange_demo.exe
.\build\exchange_matching_engine\exchange_benchmark.exe
.\build\hft_trading_platform\hft_backtest.exe


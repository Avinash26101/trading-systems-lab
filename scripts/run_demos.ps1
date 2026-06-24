$ErrorActionPreference = "Stop"

if (!(Test-Path build) -and !(Test-Path out)) {
  .\scripts\build.ps1
}

if (Test-Path build) {
  .\build\exchange_matching_engine\exchange_demo.exe
  .\build\exchange_matching_engine\exchange_benchmark.exe
  .\build\hft_trading_platform\hft_backtest.exe
} else {
  .\out\exchange\exchange_demo.exe
  .\out\exchange\exchange_benchmark.exe
  .\out\hft\hft_backtest.exe
}


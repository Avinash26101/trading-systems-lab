$ErrorActionPreference = "Stop"

if (Get-Command mingw32-make -ErrorAction SilentlyContinue) {
  cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  exit 0
}

Write-Host "mingw32-make not found; using direct g++ fallback into out/."
New-Item -ItemType Directory -Force -Path out\exchange,out\hft | Out-Null

$exchangeSrc = @(
  "exchange_matching_engine\src\order_book.cpp"
)

g++ -std=c++14 -O2 -I exchange_matching_engine\include @exchangeSrc exchange_matching_engine\src\demo.cpp -o out\exchange\exchange_demo.exe
g++ -std=c++14 -O2 -I exchange_matching_engine\include @exchangeSrc exchange_matching_engine\bench\benchmark.cpp -o out\exchange\exchange_benchmark.exe
g++ -std=c++14 -O2 -I exchange_matching_engine\include @exchangeSrc exchange_matching_engine\tests\order_book_tests.cpp -o out\exchange\exchange_tests.exe

$hftSrc = @(
  "hft_trading_platform\src\backtester.cpp",
  "hft_trading_platform\src\exchange_simulator.cpp",
  "hft_trading_platform\src\order_book.cpp",
  "hft_trading_platform\src\order_gateway.cpp",
  "hft_trading_platform\src\risk_engine.cpp",
  "hft_trading_platform\src\strategy_engine.cpp",
  "hft_trading_platform\src\udp_feed.cpp"
)

g++ -std=c++14 -O2 -I hft_trading_platform\include @hftSrc hft_trading_platform\src\backtest_main.cpp -lws2_32 -o out\hft\hft_backtest.exe
g++ -std=c++14 -O2 -I hft_trading_platform\include hft_trading_platform\src\udp_feed.cpp hft_trading_platform\src\udp_demo.cpp -lws2_32 -o out\hft\hft_udp_demo.exe
g++ -std=c++14 -O2 -I hft_trading_platform\include @hftSrc hft_trading_platform\tests\hft_tests.cpp -lws2_32 -o out\hft\hft_tests.exe

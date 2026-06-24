$ErrorActionPreference = "Stop"

if (!(Test-Path build) -and !(Test-Path out)) {
  .\scripts\build.ps1
}

if (Test-Path build) {
  ctest --test-dir build --output-on-failure
} else {
  .\out\exchange\exchange_tests.exe
  .\out\hft\hft_tests.exe
}

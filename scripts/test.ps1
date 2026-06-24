$ErrorActionPreference = "Stop"

if (!(Test-Path build)) {
  .\scripts\build.ps1
}

ctest --test-dir build --output-on-failure


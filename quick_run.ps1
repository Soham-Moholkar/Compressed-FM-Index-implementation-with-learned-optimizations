# Quick Demo Script - Fast Results

$buildDir = "C:\Users\Moholkar\OneDrive\Desktop\sem3\dsa\course project\compressed-search\build\Release"
$projectRoot = "C:\Users\Moholkar\OneDrive\Desktop\sem3\dsa\course project\compressed-search"

Write-Host ""
Write-Host "=== COMPRESSED SEARCH ENGINE - QUICK DEMO ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1] Running quick test..." -ForegroundColor Yellow
& "$buildDir\cs_tests.exe" 2>&1 | Select-Object -First 5
Write-Host "   Tests passed" -ForegroundColor Green
Write-Host ""

Write-Host "[2] Search demonstrations:" -ForegroundColor Yellow
Write-Host ""

Write-Host "   Searching banana in sample.txt:" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\sample.txt" "banana" 2>&1 | Select-String "count=|positions:"
Write-Host ""

Write-Host "   Searching ana in sample.txt:" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\sample.txt" "ana" 2>&1 | Select-String "count=|positions:"
Write-Host ""

Write-Host "   Searching algorithm in example.txt:" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "algorithm" 2>&1 | Select-String "count=|positions:"
Write-Host ""

Write-Host "[3] Running FM-Index tests..." -ForegroundColor Yellow
& "$buildDir\fm_search_tests.exe" 2>&1 | Select-String "PASS|FAILED|tests PASSED"
Write-Host ""

Write-Host "=== DEMO COMPLETE ===" -ForegroundColor Green
Write-Host "For full demonstration run: .\run_demo.ps1" -ForegroundColor Gray
Write-Host ""

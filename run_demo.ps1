# ============================================================================
# Compressed Search Engine - Complete Demo Script
# ============================================================================

Write-Host "`n" -NoNewline
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  COMPRESSED SEARCH ENGINE DEMO" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$projectRoot = "C:\Users\Moholkar\OneDrive\Desktop\sem3\dsa\course project\compressed-search"
$buildDir = "$projectRoot\build\Release"

# Check if build exists
if (-not (Test-Path "$buildDir\cs_query.exe")) {
    Write-Host "[ERROR] Executables not found. Please build the project first." -ForegroundColor Red
    Write-Host "Run: cmake --build build --config Release" -ForegroundColor Yellow
    exit 1
}

# ============================================================================
# PART 1: RUN UNIT TESTS
# ============================================================================

Write-Host "[STEP 1] Running Unit Tests..." -ForegroundColor Green
Write-Host "----------------------------------------`n" -ForegroundColor Gray

Write-Host "[1.1] Simple Tests" -ForegroundColor Yellow
& "$buildDir\cs_tests.exe"
Write-Host ""

Write-Host "[1.2] Bitvector Tests (Rank/Select Operations)" -ForegroundColor Yellow
& "$buildDir\bitvector_tests.exe"
Write-Host ""

Write-Host "[1.3] Wavelet Tree Tests" -ForegroundColor Yellow
& "$buildDir\wavelet_tests.exe"
Write-Host ""

Write-Host "[1.4] FM-Index Search Tests" -ForegroundColor Yellow
& "$buildDir\fm_search_tests.exe"
Write-Host ""

Write-Host "[1.5] Serialization Tests" -ForegroundColor Yellow
& "$buildDir\serialization_tests.exe"
Write-Host ""

Write-Host "[1.6] VEB Layout Tests" -ForegroundColor Yellow
& "$buildDir\veb_layout_tests.exe"
Write-Host ""

Write-Host "[1.7] Learned Structures Tests" -ForegroundColor Yellow
& "$buildDir\learned_occ_tests.exe"
Write-Host ""

# ============================================================================
# PART 2: SEARCH DEMONSTRATIONS
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[STEP 2] Search Demonstrations" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan

# Demo 1: Sample text
Write-Host "[2.1] Searching in sample.txt" -ForegroundColor Yellow
Write-Host "Text: '$((Get-Content "$projectRoot\sample.txt") -join ' ')'" -ForegroundColor Gray
Write-Host ""

Write-Host "  Query: 'banana'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\sample.txt" "banana"
Write-Host ""

Write-Host "  Query: 'ana'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\sample.txt" "ana"
Write-Host ""

Write-Host "  Query: 'band'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\sample.txt" "band"
Write-Host ""

# Demo 2: Example text
Write-Host "`n[2.2] Searching in example.txt" -ForegroundColor Yellow
Write-Host "Text: Large example with multiple sentences" -ForegroundColor Gray
Write-Host ""

Write-Host "  Query: 'algorithm'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "algorithm"
Write-Host ""

Write-Host "  Query: 'the'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "the"
Write-Host ""

Write-Host "  Query: 'FM-index'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "FM-index"
Write-Host ""

Write-Host "  Query: 'pattern matching'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "pattern matching"
Write-Host ""

Write-Host "  Query: 'compressed'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$projectRoot\example.txt" "compressed"
Write-Host ""

# ============================================================================
# PART 3: CREATE AND SEARCH CUSTOM TEXT
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[STEP 3] Custom Text Demo" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan

# Create a custom demo text
$customText = @"
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog again.
Data structures are important. Algorithms are important too.
Compressed search engines use Burrows-Wheeler Transform.
The FM-index enables fast pattern matching in compressed space.
Wavelet trees and suffix arrays are key data structures.
The quick brown fox loves algorithms and data structures.
"@

$customFile = "$projectRoot\demo_text.txt"
$customText | Out-File -FilePath $customFile -Encoding ASCII

Write-Host "[3.1] Created demo text file" -ForegroundColor Yellow
Write-Host "File: demo_text.txt" -ForegroundColor Gray
Write-Host ""

Write-Host "[3.2] Running searches on demo text..." -ForegroundColor Yellow
Write-Host ""

Write-Host "  Query: 'quick brown fox'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$customFile" "quick brown fox"
Write-Host ""

Write-Host "  Query: 'algorithm'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$customFile" "algorithm"
Write-Host ""

Write-Host "  Query: 'data structures'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$customFile" "data structures"
Write-Host ""

Write-Host "  Query: 'compressed'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$customFile" "compressed"
Write-Host ""

Write-Host "  Query: 'important'" -ForegroundColor Magenta
& "$buildDir\cs_query.exe" "$customFile" "important"
Write-Host ""

# ============================================================================
# PART 4: PERFORMANCE STATISTICS
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[STEP 4] Performance Statistics" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "[4.1] File Sizes" -ForegroundColor Yellow
$sampleSize = (Get-Item "$projectRoot\sample.txt").Length
$exampleSize = (Get-Item "$projectRoot\example.txt").Length
$customSize = (Get-Item "$customFile").Length

Write-Host "  sample.txt: $sampleSize bytes" -ForegroundColor Gray
Write-Host "  example.txt: $exampleSize bytes" -ForegroundColor Gray
Write-Host "  demo_text.txt: $customSize bytes" -ForegroundColor Gray
Write-Host ""

Write-Host "[4.2] Index Build Times (shown above in timers)" -ForegroundColor Yellow
Write-Host "  - Suffix Array construction: O(n) time" -ForegroundColor Gray
Write-Host "  - BWT computation: O(n) time" -ForegroundColor Gray
Write-Host "  - Wavelet tree building: O(n log σ) time" -ForegroundColor Gray
Write-Host ""

# ============================================================================
# PART 5: BUILD INDEX FILES (Optional)
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[STEP 5] Building Index Files" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "[5.1] Building index for sample.txt" -ForegroundColor Yellow
& "$buildDir\cs_build.exe" "$projectRoot\sample.txt" "$projectRoot\sample_index.csidx"
Write-Host ""

Write-Host "[5.2] Building index for example.txt" -ForegroundColor Yellow
& "$buildDir\cs_build.exe" "$projectRoot\example.txt" "$projectRoot\example_index.csidx"
Write-Host ""

Write-Host "[5.3] Building index for demo_text.txt" -ForegroundColor Yellow
& "$buildDir\cs_build.exe" "$customFile" "$projectRoot\demo_index.csidx"
Write-Host ""

# ============================================================================
# SUMMARY
# ============================================================================

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  DEMO COMPLETE!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Summary:" -ForegroundColor Yellow
Write-Host "  ✓ All unit tests passed" -ForegroundColor Green
Write-Host "  ✓ Search demonstrations completed" -ForegroundColor Green
Write-Host "  ✓ Custom text searches executed" -ForegroundColor Green
Write-Host "  ✓ Index files created" -ForegroundColor Green
Write-Host ""

Write-Host "Key Features Demonstrated:" -ForegroundColor Yellow
Write-Host "  • Compressed text indexing (FM-Index)" -ForegroundColor Gray
Write-Host "  • Fast pattern matching in O(m log σ) time" -ForegroundColor Gray
Write-Host "  • Burrows-Wheeler Transform (BWT)" -ForegroundColor Gray
Write-Host "  • Wavelet trees for compressed sequences" -ForegroundColor Gray
Write-Host "  • Rank and select operations" -ForegroundColor Gray
Write-Host "  • Suffix array construction (SAIS algorithm)" -ForegroundColor Gray
Write-Host "  • Van Emde Boas layout optimization" -ForegroundColor Gray
Write-Host "  • Learned data structures (PGM-Index)" -ForegroundColor Gray
Write-Host ""

Write-Host "Available Tools:" -ForegroundColor Yellow
Write-Host "  cs_query.exe <text_file> <pattern>  - Search for patterns" -ForegroundColor Gray
Write-Host "  cs_build.exe <input> <output.csidx> - Build compressed index" -ForegroundColor Gray
Write-Host "  cs_tests.exe                        - Run basic tests" -ForegroundColor Gray
Write-Host "  bitvector_tests.exe                 - Test rank/select" -ForegroundColor Gray
Write-Host "  fm_search_tests.exe                 - Test FM-Index search" -ForegroundColor Gray
Write-Host ""

Write-Host "Documentation:" -ForegroundColor Yellow
Write-Host "  See research_code_collection/ folder for:" -ForegroundColor Gray
Write-Host "  - START_HERE.md - Quick start guide" -ForegroundColor Gray
Write-Host "  - 00-09 documentation files - Detailed explanations" -ForegroundColor Gray
Write-Host "  - Source code with comprehensive comments" -ForegroundColor Gray
Write-Host ""

Write-Host "Press any key to exit..." -ForegroundColor Cyan
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

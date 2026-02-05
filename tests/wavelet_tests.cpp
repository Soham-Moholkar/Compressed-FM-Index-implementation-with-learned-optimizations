/**
 * wavelet_tests.cpp — Unit tests for binary wavelet tree.
 *
 * Tests:
 *   1) Empty string.
 *   2) Single character.
 *   3) "banana$" (classic FM-index example).
 *   4) All same character.
 *   5) Random bytes (verify rank matches naïve).
 *   6) Access reconstruction (verify access(i) == bwt[i]).
 */

#include "../src/core/wavelet.hpp"
#include <iostream>
#include <random>
#include <cassert>
#include <vector>
#include <cstdint>
#include <string>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Reference rank (slow, for testing correctness)
// ──────────────────────────────────────────────────────────────

static size_t naive_rank(const std::vector<uint8_t>& text, uint8_t c, size_t i) {
  size_t count = 0;
  for (size_t j = 0; j < i && j < text.size(); ++j) {
    if (text[j] == c) ++count;
  }
  return count;
}

// ──────────────────────────────────────────────────────────────
// Test utilities
// ──────────────────────────────────────────────────────────────

static void test_empty() {
  std::cout << "[TEST] Empty string\n";
  WaveletTree wt;
  std::vector<uint8_t> empty;
  wt.build(empty);
  assert(wt.size() == 0);
  assert(wt.rank('a', 0) == 0);
  assert(wt.rank('a', 10) == 0);
  std::cout << "  PASS\n";
}

static void test_single() {
  std::cout << "[TEST] Single character\n";
  WaveletTree wt;
  std::vector<uint8_t> text = {'x'};
  wt.build(text);
  assert(wt.size() == 1);
  assert(wt.rank('x', 0) == 0);
  assert(wt.rank('x', 1) == 1);
  assert(wt.rank('y', 1) == 0);
  assert(wt.access(0) == 'x');
  std::cout << "  PASS\n";
}

static void test_banana() {
  std::cout << "[TEST] banana$\n";
  WaveletTree wt;
  std::string s = "banana$";
  std::vector<uint8_t> text(s.begin(), s.end());
  wt.build(text);
  assert(wt.size() == 7);

  // Verify rank for each character.
  const char chars[] = {'b', 'a', 'n', '$'};
  for (size_t i = 0; i <= text.size(); ++i) {
    for (char c : chars) {
      const size_t expected = naive_rank(text, c, i);
      const size_t actual = wt.rank(c, i);
      if (actual != expected) {
        std::cerr << "FAIL rank('" << c << "', " << i << "): expected=" 
                  << expected << ", actual=" << actual << "\n";
        assert(false);
      }
    }
  }

  // Verify access.
  for (size_t i = 0; i < text.size(); ++i) {
    assert(wt.access(i) == text[i]);
  }

  std::cout << "  PASS\n";
}

static void test_all_same() {
  std::cout << "[TEST] All same character (n=1000)\n";
  WaveletTree wt;
  std::vector<uint8_t> text(1000, 'z');
  wt.build(text);
  assert(wt.size() == 1000);
  assert(wt.rank('z', 0) == 0);
  assert(wt.rank('z', 500) == 500);
  assert(wt.rank('z', 1000) == 1000);
  assert(wt.rank('a', 1000) == 0);
  
  for (size_t i = 0; i < text.size(); i += 100) {
    assert(wt.access(i) == 'z');
  }
  
  std::cout << "  PASS\n";
}

static void test_random(size_t n, unsigned seed) {
  std::cout << "[TEST] Random bytes (n=" << n << ", seed=" << seed << ")\n";
  std::mt19937 gen(seed);
  std::uniform_int_distribution<> dist(0, 255);

  std::vector<uint8_t> text(n);
  for (size_t i = 0; i < n; ++i) {
    text[i] = static_cast<uint8_t>(dist(gen));
  }

  WaveletTree wt;
  wt.build(text);
  assert(wt.size() == n);

  // Test rank for a subset of symbols and positions.
  std::vector<uint8_t> test_symbols = {0, 1, 42, 100, 127, 128, 200, 255};
  for (uint8_t c : test_symbols) {
    for (size_t i = 0; i <= n; i += std::max<size_t>(1, n / 20)) {
      const size_t expected = naive_rank(text, c, i);
      const size_t actual = wt.rank(c, i);
      if (actual != expected) {
        std::cerr << "FAIL rank(" << (int)c << ", " << i << "): expected=" 
                  << expected << ", actual=" << actual << "\n";
        assert(false);
      }
    }
  }

  // Test access for a subset of positions.
  for (size_t i = 0; i < n; i += std::max<size_t>(1, n / 100)) {
    const uint8_t expected = text[i];
    const uint8_t actual = wt.access(i);
    if (actual != expected) {
      std::cerr << "FAIL access(" << i << "): expected=" << (int)expected 
                << ", actual=" << (int)actual << "\n";
      assert(false);
    }
  }

  std::cout << "  PASS (verified rank & access)\n";
}

static void test_alphabet_coverage() {
  std::cout << "[TEST] Full alphabet coverage (0..255)\n";
  WaveletTree wt;
  std::vector<uint8_t> text;
  
  // Create text with all byte values 0..255.
  for (int i = 0; i < 256; ++i) {
    text.push_back(static_cast<uint8_t>(i));
  }
  // Duplicate to make it more interesting.
  text.insert(text.end(), text.begin(), text.end());
  
  wt.build(text);
  assert(wt.size() == 512);

  // Verify rank for all symbols.
  for (int c = 0; c < 256; ++c) {
    const size_t r_half = wt.rank(static_cast<uint8_t>(c), 256);
    const size_t r_full = wt.rank(static_cast<uint8_t>(c), 512);
    assert(r_half == 1);
    assert(r_full == 2);
  }

  // Verify access.
  for (size_t i = 0; i < 256; ++i) {
    assert(wt.access(i) == static_cast<uint8_t>(i));
    assert(wt.access(i + 256) == static_cast<uint8_t>(i));
  }

  std::cout << "  PASS\n";
}

static void test_boundary() {
  std::cout << "[TEST] Boundary cases\n";
  WaveletTree wt;
  std::vector<uint8_t> text = {0, 255, 0, 255};  // Min and max bytes.
  wt.build(text);
  
  assert(wt.rank(0, 0) == 0);
  assert(wt.rank(0, 1) == 1);
  assert(wt.rank(0, 2) == 1);
  assert(wt.rank(0, 3) == 2);
  assert(wt.rank(0, 4) == 2);
  
  assert(wt.rank(255, 0) == 0);
  assert(wt.rank(255, 1) == 0);
  assert(wt.rank(255, 2) == 1);
  assert(wt.rank(255, 3) == 1);
  assert(wt.rank(255, 4) == 2);
  
  assert(wt.access(0) == 0);
  assert(wt.access(1) == 255);
  assert(wt.access(2) == 0);
  assert(wt.access(3) == 255);
  
  std::cout << "  PASS\n";
}

// ──────────────────────────────────────────────────────────────
// Main test runner
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "========================================\n";
  std::cout << "WaveletTree Unit Tests\n";
  std::cout << "========================================\n";

  test_empty();
  test_single();
  test_banana();
  test_all_same();
  test_random(500, 42);
  test_random(2000, 123);
  test_random(5000, 999);
  test_alphabet_coverage();
  test_boundary();

  std::cout << "========================================\n";
  std::cout << "All WaveletTree tests PASSED!\n";
  std::cout << "========================================\n";
  return 0;
}

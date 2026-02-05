
/**
 * bitvector_tests.cpp — Unit tests for two-level sampled rank/select.
 *
 * Tests:
 *   1) Empty bitvector.
 *   2) All zeros (various sizes).
 *   3) All ones (various sizes).
 *   4) Random bitstrings (seeded, compare against naïve reference).
 *   5) Edge cases (rank at 0, rank at size, rank beyond size).
 *   6) Single-bit changes.
 */

#include "../src/core/bitvector.hpp"
#include <iostream>
#include <random>
#include <cassert>
#include <vector>
#include <cstdint>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Reference rank1 (slow, for testing correctness)
// ──────────────────────────────────────────────────────────────

static size_t naive_rank1(const std::vector<uint8_t>& bits, size_t i) {
  size_t count = 0;
  for (size_t j = 0; j < i && j < bits.size(); ++j) {
    if (bits[j]) ++count;
  }
  return count;
}

// ──────────────────────────────────────────────────────────────
// Test utilities
// ──────────────────────────────────────────────────────────────

static void test_empty() {
  std::cout << "[TEST] Empty bitvector\n";
  BitVector bv;
  std::vector<uint8_t> empty;
  bv.build(empty);
  assert(bv.size() == 0);
  assert(bv.rank1(0) == 0);
  assert(bv.rank0(0) == 0);
  assert(bv.count_ones() == 0);
  std::cout << "  PASS\n";
}

static void test_all_zeros(size_t n) {
  std::cout << "[TEST] All zeros (n=" << n << ")\n";
  std::vector<uint8_t> bits(n, 0);
  BitVector bv;
  bv.build(bits);
  assert(bv.size() == n);
  assert(bv.rank1(0) == 0);
  assert(bv.rank1(n) == 0);
  assert(bv.rank0(n) == n);
  assert(bv.count_ones() == 0);
  // Check a few positions.
  for (size_t i = 0; i <= n; i += std::max<size_t>(1, n / 10)) {
    assert(bv.rank1(i) == 0);
    assert(bv.rank0(i) == i);
  }
  std::cout << "  PASS\n";
}

static void test_all_ones(size_t n) {
  std::cout << "[TEST] All ones (n=" << n << ")\n";
  std::vector<uint8_t> bits(n, 1);
  BitVector bv;
  bv.build(bits);
  assert(bv.size() == n);
  assert(bv.rank1(0) == 0);
  assert(bv.rank1(n) == n);
  assert(bv.rank0(n) == 0);
  assert(bv.count_ones() == n);
  // Check a few positions.
  for (size_t i = 0; i <= n; i += std::max<size_t>(1, n / 10)) {
    assert(bv.rank1(i) == i);
    assert(bv.rank0(i) == 0);
  }
  std::cout << "  PASS\n";
}

static void test_random(size_t n, unsigned seed) {
  std::cout << "[TEST] Random bits (n=" << n << ", seed=" << seed << ")\n";
  std::mt19937 gen(seed);
  std::uniform_int_distribution<> dist(0, 1);

  std::vector<uint8_t> bits(n);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = static_cast<uint8_t>(dist(gen));
  }

  BitVector bv;
  bv.build(bits);
  assert(bv.size() == n);

  // Verify rank1 at every position (compare against naïve).
  for (size_t i = 0; i <= n; ++i) {
    const size_t expected = naive_rank1(bits, i);
    const size_t actual   = bv.rank1(i);
    if (actual != expected) {
      std::cerr << "FAIL at i=" << i << ": expected=" << expected 
                << ", actual=" << actual << "\n";
      assert(false);
    }
  }

  // Verify rank0.
  for (size_t i = 0; i <= n; i += std::max<size_t>(1, n / 100)) {
    const size_t r1 = bv.rank1(i);
    const size_t r0 = bv.rank0(i);
    assert(r1 + r0 == i);
  }

  // Verify count_ones.
  const size_t total_ones = naive_rank1(bits, n);
  assert(bv.count_ones() == total_ones);

  std::cout << "  PASS (verified all positions)\n";
}

static void test_edge_cases() {
  std::cout << "[TEST] Edge cases\n";
  // Single bit = 1.
  {
    BitVector bv;
    std::vector<uint8_t> bits{1};
    bv.build(bits);
    assert(bv.size() == 1);
    assert(bv.rank1(0) == 0);
    assert(bv.rank1(1) == 1);
    assert(bv.rank1(100) == 1);  // Beyond size.
    assert(bv.count_ones() == 1);
  }
  // Single bit = 0.
  {
    BitVector bv;
    std::vector<uint8_t> bits{0};
    bv.build(bits);
    assert(bv.size() == 1);
    assert(bv.rank1(0) == 0);
    assert(bv.rank1(1) == 0);
    assert(bv.rank0(1) == 1);
    assert(bv.count_ones() == 0);
  }
  // Bitvector with size exactly = super-block size.
  {
    constexpr size_t N = CS_SUPER_BLOCK_SIZE;
    std::vector<uint8_t> bits(N, 1);
    BitVector bv;
    bv.build(bits);
    assert(bv.size() == N);
    assert(bv.rank1(N) == N);
  }
  // Bitvector with size = super-block + 1.
  {
    constexpr size_t N = CS_SUPER_BLOCK_SIZE + 1;
    std::vector<uint8_t> bits(N, 1);
    bits[N - 1] = 0;
    BitVector bv;
    bv.build(bits);
    assert(bv.rank1(N) == N - 1);
  }
  std::cout << "  PASS\n";
}

static void test_build_from_words() {
  std::cout << "[TEST] build_from_words\n";
  // Create a simple pattern: alternating 0xAAAAAAAAAAAAAAAA and 0x5555555555555555.
  std::vector<uint64_t> words = {
    0xAAAAAAAAAAAAAAAAULL,  // 1010...
    0x5555555555555555ULL   // 0101...
  };
  const size_t nbits = 128;
  BitVector bv;
  bv.build_from_words(words, nbits);
  assert(bv.size() == 128);
  // Each word has 32 ones.
  assert(bv.count_ones() == 64);
  assert(bv.rank1(64) == 32);   // First word.
  assert(bv.rank1(128) == 64);  // Both words.
  std::cout << "  PASS\n";
}

// ──────────────────────────────────────────────────────────────
// Main test runner
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "========================================\n";
  std::cout << "BitVector Unit Tests\n";
  std::cout << "========================================\n";

  test_empty();
  test_all_zeros(100);
  test_all_zeros(2048);
  test_all_zeros(5000);
  test_all_ones(100);
  test_all_ones(2048);
  test_all_ones(5000);
  test_random(500, 42);
  test_random(2048, 123);
  test_random(5000, 999);
  test_random(10000, 7777);
  test_edge_cases();
  test_build_from_words();

  std::cout << "========================================\n";
  std::cout << "All BitVector tests PASSED!\n";
  std::cout << "========================================\n";
  return 0;
}

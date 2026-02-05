/**
 * learned_occ_tests.cpp — Tests for learned bitvector with PGM.
 */

#include "../src/core/bitvector_learned.hpp"
#include "../src/core/bitvector.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cassert>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Helper: Naïve rank for reference
// ──────────────────────────────────────────────────────────────

static size_t naive_rank(const std::vector<uint8_t>& bits, size_t i) {
  size_t r = 0;
  for (size_t k = 0; k < i && k < bits.size(); ++k) {
    if (bits[k]) ++r;
  }
  return r;
}

// ──────────────────────────────────────────────────────────────
// Test 1: Small bitvector with all zeros
// ──────────────────────────────────────────────────────────────

static void test_all_zeros() {
  std::cout << "[learned_occ_tests] Test 1: All zeros\n";
  const size_t n = 1024;
  std::vector<uint8_t> bits(n, 0);

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  for (size_t i = 0; i <= n; i += 100) {
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "All zeros rank mismatch");
  }
  std::cout << "  ✓ All zeros passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 2: Small bitvector with all ones
// ──────────────────────────────────────────────────────────────

static void test_all_ones() {
  std::cout << "[learned_occ_tests] Test 2: All ones\n";
  const size_t n = 1024;
  std::vector<uint8_t> bits(n, 1);

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  for (size_t i = 0; i <= n; i += 100) {
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "All ones rank mismatch");
  }
  std::cout << "  ✓ All ones passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 3: Random bitvector with default strides (S=512, s=32)
// ──────────────────────────────────────────────────────────────

static void test_random_default_strides() {
  std::cout << "[learned_occ_tests] Test 3: Random bitvector (S=512, s=32)\n";
  const size_t n = 5000;
  std::vector<uint8_t> bits(n);

  std::srand(42);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = (std::rand() % 2);
  }

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  // Test at 100 random positions.
  std::srand(42);
  for (int t = 0; t < 100; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Random default strides rank mismatch");
  }
  std::cout << "  ✓ Random (S=512, s=32) passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 4: Random bitvector with smaller strides (S=256, s=16)
// ──────────────────────────────────────────────────────────────

static void test_random_small_strides() {
  std::cout << "[learned_occ_tests] Test 4: Random bitvector (S=256, s=16)\n";
  const size_t n = 3000;
  std::vector<uint8_t> bits(n);

  std::srand(99);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = (std::rand() % 2);
  }

  BitvectorLearned bv;
  bv.build(bits, 256, 16);

  std::srand(99);
  for (int t = 0; t < 100; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Random small strides rank mismatch");
  }
  std::cout << "  ✓ Random (S=256, s=16) passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 5: Compare learned vs classic bitvector
// ──────────────────────────────────────────────────────────────

static void test_vs_classic_bitvector() {
  std::cout << "[learned_occ_tests] Test 5: Learned vs Classic BitVector\n";
  const size_t n = 8000;
  std::vector<uint8_t> bits(n);

  std::srand(123);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = (std::rand() % 2);
  }

  // Build classic bitvector.
  BitVector classic;
  classic.build(bits);

  // Build learned bitvector.
  BitvectorLearned learned;
  learned.build(bits, 512, 32);

  // Compare at 200 random positions.
  std::srand(123);
  for (int t = 0; t < 200; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t classic_rank = classic.rank1(i);
    const size_t learned_rank = learned.rank1(i);
    assert(classic_rank == learned_rank && "Learned vs classic rank mismatch");
  }
  std::cout << "  ✓ Learned matches Classic passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 6: Boundary cases
// ──────────────────────────────────────────────────────────────

static void test_boundaries() {
  std::cout << "[learned_occ_tests] Test 6: Boundary cases\n";
  const size_t n = 2048;
  std::vector<uint8_t> bits(n);

  std::srand(77);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = (std::rand() % 2);
  }

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  // Test boundaries: i=0, i=nbits, coarse/micro boundaries.
  std::vector<size_t> test_positions = {0, 1, 31, 32, 33, 511, 512, 513, 1023, 1024, 1536, 2047, 2048};
  for (size_t i : test_positions) {
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Boundary rank mismatch");
  }
  std::cout << "  ✓ Boundary cases passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 7: Large bitvector
// ──────────────────────────────────────────────────────────────

static void test_large_bitvector() {
  std::cout << "[learned_occ_tests] Test 7: Large bitvector (50K bits)\n";
  const size_t n = 50000;
  std::vector<uint8_t> bits(n);

  std::srand(2024);
  for (size_t i = 0; i < n; ++i) {
    bits[i] = (std::rand() % 2);
  }

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  // Sample 500 random positions.
  std::srand(2024);
  for (int t = 0; t < 500; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Large bitvector rank mismatch");
  }
  std::cout << "  ✓ Large bitvector passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 8: Sparse bitvector (few ones)
// ──────────────────────────────────────────────────────────────

static void test_sparse() {
  std::cout << "[learned_occ_tests] Test 8: Sparse bitvector\n";
  const size_t n = 10000;
  std::vector<uint8_t> bits(n, 0);

  // Set ~1% of bits to 1.
  std::srand(555);
  for (int i = 0; i < 100; ++i) {
    const size_t pos = std::rand() % n;
    bits[pos] = 1;
  }

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  std::srand(555);
  for (int t = 0; t < 200; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Sparse rank mismatch");
  }
  std::cout << "  ✓ Sparse bitvector passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 9: Dense bitvector (few zeros)
// ──────────────────────────────────────────────────────────────

static void test_dense() {
  std::cout << "[learned_occ_tests] Test 9: Dense bitvector\n";
  const size_t n = 10000;
  std::vector<uint8_t> bits(n, 1);

  // Clear ~1% of bits.
  std::srand(666);
  for (int i = 0; i < 100; ++i) {
    const size_t pos = std::rand() % n;
    bits[pos] = 0;
  }

  BitvectorLearned bv;
  bv.build(bits, 512, 32);

  std::srand(666);
  for (int t = 0; t < 200; ++t) {
    const size_t i = std::rand() % (n + 1);
    const size_t expected = naive_rank(bits, i);
    const size_t got = bv.rank1(i);
    assert(got == expected && "Dense rank mismatch");
  }
  std::cout << "  ✓ Dense bitvector passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 10: Empty bitvector
// ──────────────────────────────────────────────────────────────

static void test_empty() {
  std::cout << "[learned_occ_tests] Test 10: Empty bitvector\n";
  std::vector<uint8_t> bits;

  BitvectorLearned bv;
  bv.build(bits);

  assert(bv.rank1(0) == 0 && "Empty rank(0) should be 0");
  assert(bv.count_ones() == 0 && "Empty count_ones should be 0");
  std::cout << "  ✓ Empty bitvector passed\n";
}

// ──────────────────────────────────────────────────────────────
// Main test driver
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "=== Running learned_occ_tests ===\n";

  test_all_zeros();
  test_all_ones();
  test_random_default_strides();
  test_random_small_strides();
  test_vs_classic_bitvector();
  test_boundaries();
  test_large_bitvector();
  test_sparse();
  test_dense();
  test_empty();

  std::cout << "=== All learned_occ_tests passed! ===\n";
  return 0;
}

/**
 * veb_layout_tests.cpp — Tests for van Emde Boas layout.
 */

#include "../src/layout/veb.hpp"
#include "../src/core/bitvector.hpp"
#include <iostream>
#include <vector>
#include <cassert>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Test 1: Simple 2-level wavelet tree
// ──────────────────────────────────────────────────────────────

static void test_two_levels() {
  std::cout << "[veb_layout_tests] Test 1: Two-level tree\n";

  // Create two simple bit vectors.
  std::vector<uint8_t> bits1 = {1, 0, 1, 1, 0, 0, 1, 0};
  std::vector<uint8_t> bits2 = {0, 1, 1, 0, 1, 0, 0, 1};

  BitVector bv1, bv2;
  bv1.build(bits1);
  bv2.build(bits2);

  BitVector levels[2] = {bv1, bv2};

  // Build vEB layout.
  VebLayout veb;
  veb.build(levels, 2, 2);  // top_k=2, so both inline

  // Verify we can get offsets.
  const size_t off0 = veb.get_level_offset(0);
  const size_t off1 = veb.get_level_offset(1);

  assert(off0 < veb.size() && "Level 0 offset should be valid");
  assert(off1 < veb.size() && "Level 1 offset should be valid");
  assert(off1 > off0 && "Level 1 should come after level 0");

  std::cout << "  ✓ Two-level tree passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 2: Full 8-level wavelet tree
// ──────────────────────────────────────────────────────────────

static void test_eight_levels() {
  std::cout << "[veb_layout_tests] Test 2: Eight-level tree\n";

  const size_t num_levels = 8;
  BitVector levels[8];

  // Create 8 levels with different sizes.
  for (size_t i = 0; i < num_levels; ++i) {
    const size_t n = 100 + i * 10;
    std::vector<uint8_t> bits(n);
    for (size_t j = 0; j < n; ++j) {
      bits[j] = (j % 3 == 0) ? 1 : 0;
    }
    levels[i].build(bits);
  }

  // Build vEB layout with top_k=2.
  VebLayout veb;
  veb.build(levels, num_levels, 2);

  // Verify all level offsets are accessible.
  for (size_t i = 0; i < num_levels; ++i) {
    const size_t off = veb.get_level_offset(i);
    assert(off < veb.size() && "Level offset should be valid");
    
    const uint8_t* ptr = veb.level_data(i);
    assert(ptr != nullptr && "Level data pointer should be valid");
  }

  // Verify 4KB alignment for bottom levels.
  for (size_t i = 2; i < num_levels; ++i) {
    const size_t off = veb.get_level_offset(i);
    assert(off % VEB_MACROBLOCK_SIZE == 0 && "Bottom level should be 4KB-aligned");
  }

  std::cout << "  ✓ Eight-level tree passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 3: Empty bitvector levels
// ──────────────────────────────────────────────────────────────

static void test_empty_levels() {
  std::cout << "[veb_layout_tests] Test 3: Empty levels\n";

  BitVector levels[3];
  std::vector<uint8_t> empty;

  levels[0].build(empty);
  levels[1].build(empty);
  levels[2].build(empty);

  VebLayout veb;
  veb.build(levels, 3, 2);

  // Should handle empty levels gracefully.
  assert(veb.size() > 0 && "vEB layout should have some data (padding)");
  assert(veb.size() % VEB_MACROBLOCK_SIZE == 0 && "Should be 4KB-aligned");

  std::cout << "  ✓ Empty levels passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 4: Large bitvectors (test alignment and padding)
// ──────────────────────────────────────────────────────────────

static void test_large_bitvectors() {
  std::cout << "[veb_layout_tests] Test 4: Large bitvectors\n";

  const size_t num_levels = 5;
  BitVector levels[5];

  // Create large levels (10K bits each).
  for (size_t i = 0; i < num_levels; ++i) {
    const size_t n = 10000;
    std::vector<uint8_t> bits(n);
    for (size_t j = 0; j < n; ++j) {
      bits[j] = ((j + i) % 7 == 0) ? 1 : 0;
    }
    levels[i].build(bits);
  }

  VebLayout veb;
  veb.build(levels, num_levels, 2);

  // Verify final size is 4KB-aligned.
  assert(veb.size() % VEB_MACROBLOCK_SIZE == 0 && "Total size should be 4KB-aligned");

  // Verify all bottom levels are 4KB-aligned.
  for (size_t i = 2; i < num_levels; ++i) {
    const size_t off = veb.get_level_offset(i);
    assert(off % VEB_MACROBLOCK_SIZE == 0 && "Bottom level should be 4KB-aligned");
  }

  std::cout << "  ✓ Large bitvectors passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 5: Single level (edge case)
// ──────────────────────────────────────────────────────────────

static void test_single_level() {
  std::cout << "[veb_layout_tests] Test 5: Single level\n";

  std::vector<uint8_t> bits = {1, 0, 1, 0, 1, 1, 0, 0};
  BitVector bv;
  bv.build(bits);

  BitVector levels[1] = {bv};

  VebLayout veb;
  veb.build(levels, 1, 1);

  const size_t off = veb.get_level_offset(0);
  assert(off == 0 && "Single level should start at offset 0");
  assert(veb.size() > 0 && "Should have data");

  std::cout << "  ✓ Single level passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 6: Verify data integrity (serialize and check)
// ──────────────────────────────────────────────────────────────

static void test_data_integrity() {
  std::cout << "[veb_layout_tests] Test 6: Data integrity\n";

  const size_t num_levels = 4;
  BitVector levels[4];

  // Create known patterns.
  for (size_t i = 0; i < num_levels; ++i) {
    const size_t n = 128;
    std::vector<uint8_t> bits(n, 0);
    // Set every (i+1)-th bit.
    for (size_t j = i; j < n; j += (i + 1)) {
      bits[j] = 1;
    }
    levels[i].build(bits);
  }

  VebLayout veb;
  veb.build(levels, num_levels, 2);

  // Verify we can access level data pointers.
  for (size_t i = 0; i < num_levels; ++i) {
    const uint8_t* ptr = veb.level_data(i);
    assert(ptr != nullptr && "Level data should be accessible");
    
    // First 8 bytes should be nbits.
    size_t nbits = *reinterpret_cast<const size_t*>(ptr);
    assert(nbits == 128 && "nbits should match original size");
  }

  std::cout << "  ✓ Data integrity passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 7: Different top_k values
// ──────────────────────────────────────────────────────────────

static void test_different_top_k() {
  std::cout << "[veb_layout_tests] Test 7: Different top_k values\n";

  const size_t num_levels = 6;
  BitVector levels[6];

  for (size_t i = 0; i < num_levels; ++i) {
    std::vector<uint8_t> bits(50 + i * 5);
    for (size_t j = 0; j < bits.size(); ++j) {
      bits[j] = (j % 2);
    }
    levels[i].build(bits);
  }

  // Test with top_k=1, 2, 3.
  for (size_t top_k = 1; top_k <= 3; ++top_k) {
    VebLayout veb;
    veb.build(levels, num_levels, top_k);

    // Verify bottom levels are 4KB-aligned.
    for (size_t i = top_k; i < num_levels; ++i) {
      const size_t off = veb.get_level_offset(i);
      assert(off % VEB_MACROBLOCK_SIZE == 0 && "Bottom level should be 4KB-aligned");
    }
  }

  std::cout << "  ✓ Different top_k values passed\n";
}

// ──────────────────────────────────────────────────────────────
// Main test driver
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "=== Running veb_layout_tests ===\n";

  test_two_levels();
  test_eight_levels();
  test_empty_levels();
  test_large_bitvectors();
  test_single_level();
  test_data_integrity();
  test_different_top_k();

  std::cout << "=== All veb_layout_tests passed! ===\n";
  return 0;
}

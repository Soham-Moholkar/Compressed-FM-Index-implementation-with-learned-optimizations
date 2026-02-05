/**
 * fm_search_tests.cpp — Unit tests for FM-index count/locate operations.
 *
 * Tests:
 *   1) Empty text/pattern.
 *   2) "banana$" (classic example).
 *   3) No matches.
 *   4) Multiple matches.
 *   5) Overlapping matches.
 *   6) Random text with known patterns.
 */

#include "../src/api/fm_index.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string>
#include <set>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Naive reference implementation
// ──────────────────────────────────────────────────────────────

static size_t naive_count(const std::string& text, const std::string& pattern) {
  if (pattern.empty()) return text.size();
  size_t count = 0;
  for (size_t i = 0; i + pattern.size() <= text.size(); ++i) {
    if (text.compare(i, pattern.size(), pattern) == 0) {
      ++count;
    }
  }
  return count;
}

static std::vector<uint64_t> naive_locate(const std::string& text, const std::string& pattern) {
  std::vector<uint64_t> positions;
  if (pattern.empty()) return positions;
  for (size_t i = 0; i + pattern.size() <= text.size(); ++i) {
    if (text.compare(i, pattern.size(), pattern) == 0) {
      positions.push_back(i);
    }
  }
  return positions;
}

// ──────────────────────────────────────────────────────────────
// Test utilities
// ──────────────────────────────────────────────────────────────

static void test_empty() {
  std::cout << "[TEST] Empty text/pattern\n";
  
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text("", params);
  assert(idx.count("") == 0);
  assert(idx.count("x") == 0);
  assert(idx.locate("x").empty());
  
  FMIndex idx2 = FMIndex::build_from_text("hello$", params);
  // Empty pattern: by convention should match n+1 positions (after each character + end).
  assert(idx2.count("") > 0);
  
  std::cout << "  PASS\n";
}

static void test_banana() {
  std::cout << "[TEST] banana$\n";
  
  std::string text = "banana$";
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  // Test count.
  assert(idx.count("banana") == 1);
  assert(idx.count("ana") == 2);
  assert(idx.count("na") == 2);
  assert(idx.count("a") == 3);
  assert(idx.count("b") == 1);
  assert(idx.count("$") == 1);
  assert(idx.count("x") == 0);
  assert(idx.count("anana") == 1);
  
  // Test locate.
  {
    auto pos = idx.locate("banana");
    assert(pos.size() == 1);
    assert(pos[0] == 0);
  }
  {
    auto pos = idx.locate("ana");
    assert(pos.size() == 2);
    std::sort(pos.begin(), pos.end());
    assert(pos[0] == 1);
    assert(pos[1] == 3);
  }
  {
    auto pos = idx.locate("a");
    assert(pos.size() == 3);
    std::sort(pos.begin(), pos.end());
    assert(pos[0] == 1);
    assert(pos[1] == 3);
    assert(pos[2] == 5);
  }
  {
    auto pos = idx.locate("x");
    assert(pos.empty());
  }
  
  std::cout << "  PASS\n";
}

static void test_no_match() {
  std::cout << "[TEST] No matches\n";
  
  std::string text = "abcdefg$";
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  assert(idx.count("xyz") == 0);
  assert(idx.count("aaa") == 0);
  assert(idx.count("gg") == 0);
  assert(idx.locate("xyz").empty());
  
  std::cout << "  PASS\n";
}

static void test_multiple_matches() {
  std::cout << "[TEST] Multiple matches\n";
  
  // Use a text with terminator
  std::string text = "aabaabaa$";
  BuildParams params;
  params.ssa_stride = 4;  // Reasonable stride
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  const size_t count_a = idx.count("a");
  const size_t expected_a = naive_count(text, "a");
  assert(count_a == expected_a);
  
  const size_t count_aa = idx.count("aa");
  const size_t expected_aa = naive_count(text, "aa");
  assert(count_aa == expected_aa);
  
  const size_t count_aab = idx.count("aab");
  const size_t expected_aab = naive_count(text, "aab");
  assert(count_aab == expected_aab);
  
  std::cout << "  PASS\n";
}

static void test_overlapping() {
  std::cout << "[TEST] Overlapping matches\n";
  
  std::string text = "abababab$";
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  assert(idx.count("ab") == 4);
  assert(idx.count("aba") == 3);
  assert(idx.count("abab") == 3);
  
  auto pos = idx.locate("aba");
  assert(pos.size() == 3);
  std::sort(pos.begin(), pos.end());
  assert(pos[0] == 0);
  assert(pos[1] == 2);
  assert(pos[2] == 4);
  
  std::cout << "  PASS\n";
}

static void test_full_alphabet() {
  std::cout << "[TEST] Full byte alphabet\n";
  
  std::string text;
  // Use bytes 1-255 (skip 0 to reserve for potential terminator).
  for (int i = 1; i < 256; ++i) {
    text += static_cast<char>(i);
  }
  text += '$';  // Add terminator.
  
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  // Each byte (1-255) appears exactly once.
  for (int i = 1; i < 256; ++i) {
    std::string pattern(1, static_cast<char>(i));
    assert(idx.count(pattern) == 1);
    auto pos = idx.locate(pattern);
    assert(pos.size() == 1);
    assert(pos[0] == i - 1);  // Position in text (0-indexed).
  }
  
  std::cout << "  PASS\n";
}

static void test_against_naive(const std::string& text, const std::vector<std::string>& patterns) {
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  for (const auto& pattern : patterns) {
    // Test count.
    const size_t expected_count = naive_count(text, pattern);
    const size_t actual_count = idx.count(pattern);
    if (actual_count != expected_count) {
      std::cerr << "FAIL count(\"" << pattern << "\"): expected=" << expected_count
                << ", actual=" << actual_count << "\n";
      assert(false);
    }
    
    // Test locate.
    auto expected_pos = naive_locate(text, pattern);
    auto actual_pos = idx.locate(pattern);
    
    if (actual_pos.size() != expected_pos.size()) {
      std::cerr << "FAIL locate(\"" << pattern << "\"): expected " << expected_pos.size()
                << " positions, got " << actual_pos.size() << "\n";
      assert(false);
    }
    
    std::sort(expected_pos.begin(), expected_pos.end());
    std::sort(actual_pos.begin(), actual_pos.end());
    
    for (size_t i = 0; i < expected_pos.size(); ++i) {
      if (actual_pos[i] != expected_pos[i]) {
        std::cerr << "FAIL locate(\"" << pattern << "\")[" << i << "]: expected="
                  << expected_pos[i] << ", actual=" << actual_pos[i] << "\n";
        assert(false);
      }
    }
  }
}

static void test_long_text() {
  std::cout << "[TEST] Long text with various patterns\n";
  
  std::string text = "The quick brown fox jumps over the lazy dog. ";
  text += "The five boxing wizards jump quickly. ";
  text += "Pack my box with five dozen liquor jugs.$";
  
  std::vector<std::string> patterns = {
    "The", "the", "quick", "fox", "dog", "jump", "five", "box", "xyz",
    " ", ".", "qu", "ing", "ck", "ox"
  };
  
  test_against_naive(text, patterns);
  std::cout << "  PASS\n";
}

static void test_repeated_pattern() {
  std::cout << "[TEST] Repeated pattern\n";
  
  std::string text = "abcabcabcabc$";
  std::vector<std::string> patterns = {"abc", "ab", "bc", "ca", "abcabc", "a", "c"};
  
  test_against_naive(text, patterns);
  std::cout << "  PASS\n";
}

static void test_single_char() {
  std::cout << "[TEST] Single character text\n";
  
  std::string text = "x$";
  BuildParams params;
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  assert(idx.count("x") == 1);
  assert(idx.count("y") == 0);
  
  auto pos = idx.locate("x");
  assert(pos.size() == 1);
  assert(pos[0] == 0);
  
  std::cout << "  PASS\n";
}

// ──────────────────────────────────────────────────────────────
// Main test runner
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "========================================\n";
  std::cout << "FM-Index Search Tests\n";
  std::cout << "========================================\n";

  test_empty();
  test_banana();
  test_no_match();
  test_multiple_matches();
  test_overlapping();
  test_full_alphabet();
  test_long_text();
  test_repeated_pattern();
  test_single_char();

  std::cout << "========================================\n";
  std::cout << "All FM-Index search tests PASSED!\n";
  std::cout << "========================================\n";
  return 0;
}

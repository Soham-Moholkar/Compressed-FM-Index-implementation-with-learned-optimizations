/**
 * serialization_tests.cpp — Tests for index serialization/deserialization.
 */

#include "../src/serialization/serialization.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cstdio>

using namespace cs;

// Test file path
const std::string TEST_INDEX_PATH = "test_index.csidx";

// ──────────────────────────────────────────────────────────────
// Helper: Clean up test file
// ──────────────────────────────────────────────────────────────

static void cleanup_test_file() {
  std::remove(TEST_INDEX_PATH.c_str());
}

// ──────────────────────────────────────────────────────────────
// Test 1: Write and read header
// ──────────────────────────────────────────────────────────────

static void test_header_roundtrip() {
  std::cout << "[serialization_tests] Test 1: Header roundtrip\n";
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_VEB_LAYOUT | FLAG_LEARNED_OCC, 12345);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    const IndexHeader* hdr = reader.header();
    
    assert(hdr != nullptr && "Header should not be null");
    assert(hdr->is_valid() && "Header should be valid");
    assert(hdr->flags == (FLAG_VEB_LAYOUT | FLAG_LEARNED_OCC) && "Flags mismatch");
    assert(hdr->text_len == 12345 && "Text length mismatch");
    assert(reader.has_flag(FLAG_VEB_LAYOUT) && "Should have VEB flag");
    assert(reader.has_flag(FLAG_LEARNED_OCC) && "Should have learned occ flag");
  }
  
  cleanup_test_file();
  std::cout << "  ✓ Header roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 2: Write and read text
// ──────────────────────────────────────────────────────────────

static void test_text_roundtrip() {
  std::cout << "[serialization_tests] Test 2: Text roundtrip\n";
  
  const std::string original_text = "hello world$";
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_NONE, original_text.size());
    writer.write_text(original_text);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t len = 0;
    const char* text = reader.get_text(&len);
    
    assert(text != nullptr && "Text should not be null");
    assert(len == original_text.size() && "Text length mismatch");
    assert(std::string(text, len) == original_text && "Text content mismatch");
  }
  
  cleanup_test_file();
  std::cout << "  ✓ Text roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 3: Write and read BWT
// ──────────────────────────────────────────────────────────────

static void test_bwt_roundtrip() {
  std::cout << "[serialization_tests] Test 3: BWT roundtrip\n";
  
  std::vector<uint8_t> bwt = {3, 1, 4, 1, 5, 9, 2, 6};
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_NONE, bwt.size());
    writer.write_bwt(bwt);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t len = 0;
    const uint8_t* bwt_read = reader.get_bwt(&len);
    
    assert(bwt_read != nullptr && "BWT should not be null");
    assert(len == bwt.size() && "BWT length mismatch");
    for (size_t i = 0; i < len; ++i) {
      assert(bwt_read[i] == bwt[i] && "BWT content mismatch");
    }
  }
  
  cleanup_test_file();
  std::cout << "  ✓ BWT roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 4: Write and read C-array
// ──────────────────────────────────────────────────────────────

static void test_c_array_roundtrip() {
  std::cout << "[serialization_tests] Test 4: C-array roundtrip\n";
  
  std::vector<uint32_t> c_array(256, 0);
  for (size_t i = 0; i < 256; ++i) {
    c_array[i] = i * 100;
  }
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_NONE, 1000);
    writer.write_c_array(c_array);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t len = 0;
    const uint32_t* c_read = reader.get_c_array(&len);
    
    assert(c_read != nullptr && "C-array should not be null");
    assert(len == c_array.size() && "C-array length mismatch");
    for (size_t i = 0; i < len; ++i) {
      assert(c_read[i] == c_array[i] && "C-array content mismatch");
    }
  }
  
  cleanup_test_file();
  std::cout << "  ✓ C-array roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 5: Write and read SSA
// ──────────────────────────────────────────────────────────────

static void test_ssa_roundtrip() {
  std::cout << "[serialization_tests] Test 5: SSA roundtrip\n";
  
  std::vector<uint32_t> ssa_samples = {0, 32, 64, 96, 128};
  uint32_t stride = 32;
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_NONE, 160);
    writer.write_ssa(ssa_samples, stride);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t len = 0;
    uint32_t read_stride = 0;
    const uint32_t* ssa_read = reader.get_ssa(&len, &read_stride);
    
    assert(ssa_read != nullptr && "SSA should not be null");
    assert(read_stride == stride && "SSA stride mismatch");
    assert(len == ssa_samples.size() && "SSA length mismatch");
    for (size_t i = 0; i < len; ++i) {
      assert(ssa_read[i] == ssa_samples[i] && "SSA content mismatch");
    }
  }
  
  cleanup_test_file();
  std::cout << "  ✓ SSA roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 6: Write and read wavelet
// ──────────────────────────────────────────────────────────────

static void test_wavelet_roundtrip() {
  std::cout << "[serialization_tests] Test 6: Wavelet roundtrip\n";
  
  std::vector<uint64_t> bits_data = {0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL};
  std::vector<uint32_t> super_data = {0, 32, 64};
  std::vector<uint16_t> sub_data = {0, 8, 16, 24};
  size_t num_levels = 8;
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_NONE, 100);
    writer.write_wavelet(bits_data, super_data, sub_data, num_levels);
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t size = 0;
    const uint8_t* wavelet_ptr = reader.get_wavelet(&size);
    
    assert(wavelet_ptr != nullptr && "Wavelet should not be null");
    assert(size > 0 && "Wavelet size should be positive");
    
    // Verify num_levels
    const uint64_t* levels_ptr = reinterpret_cast<const uint64_t*>(wavelet_ptr);
    assert(*levels_ptr == num_levels && "Num levels mismatch");
  }
  
  cleanup_test_file();
  std::cout << "  ✓ Wavelet roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 7: Write and read vEB layout
// ──────────────────────────────────────────────────────────────

static void test_veb_layout_roundtrip() {
  std::cout << "[serialization_tests] Test 7: vEB layout roundtrip\n";
  
  // Create dummy vEB data (4KB aligned)
  std::vector<uint8_t> veb_data(4096, 0);
  for (size_t i = 0; i < veb_data.size(); ++i) {
    veb_data[i] = static_cast<uint8_t>(i % 256);
  }
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_VEB_LAYOUT, 1000);
    writer.write_veb_layout(veb_data.data(), veb_data.size());
    writer.finalize();
  }
  
  // Read
  {
    IndexReader reader(TEST_INDEX_PATH);
    size_t len = 0;
    const uint8_t* veb_read = reader.get_veb_layout(&len);
    
    assert(veb_read != nullptr && "vEB layout should not be null");
    assert(len == veb_data.size() && "vEB layout length mismatch");
    for (size_t i = 0; i < len; ++i) {
      assert(veb_read[i] == veb_data[i] && "vEB layout content mismatch");
    }
  }
  
  cleanup_test_file();
  std::cout << "  ✓ vEB layout roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Test 8: Full index roundtrip
// ──────────────────────────────────────────────────────────────

static void test_full_index() {
  std::cout << "[serialization_tests] Test 8: Full index roundtrip\n";
  
  const std::string text = "banana$";
  std::vector<uint8_t> bwt = {36, 97, 110, 110, 98, 97, 97};  // BWT of "banana$"
  std::vector<uint32_t> c_array(256, 0);
  c_array[36] = 0;  // '$'
  c_array[97] = 1;  // 'a'
  c_array[98] = 4;  // 'b'
  c_array[110] = 5; // 'n'
  
  std::vector<uint32_t> ssa_samples = {0, 2, 4, 6};
  uint32_t stride = 2;
  
  std::vector<uint64_t> bits_data = {0xABCD};
  std::vector<uint32_t> super_data = {0};
  std::vector<uint16_t> sub_data = {0};
  size_t num_levels = 8;
  
  // Write
  {
    IndexWriter writer(TEST_INDEX_PATH);
    writer.write_header(FLAG_VEB_LAYOUT, text.size());
    writer.write_text(text);
    writer.write_bwt(bwt);
    writer.write_c_array(c_array);
    writer.write_ssa(ssa_samples, stride);
    writer.write_wavelet(bits_data, super_data, sub_data, num_levels);
    writer.finalize();
  }
  
  // Read and verify all sections
  {
    IndexReader reader(TEST_INDEX_PATH);
    
    // Header
    assert(reader.has_flag(FLAG_VEB_LAYOUT) && "Should have VEB flag");
    assert(reader.header()->text_len == text.size() && "Text length mismatch");
    
    // Text
    size_t text_len = 0;
    const char* text_read = reader.get_text(&text_len);
    assert(std::string(text_read, text_len) == text && "Text mismatch");
    
    // BWT
    size_t bwt_len = 0;
    const uint8_t* bwt_read = reader.get_bwt(&bwt_len);
    assert(bwt_len == bwt.size() && "BWT length mismatch");
    
    // C-array
    size_t c_len = 0;
    const uint32_t* c_read = reader.get_c_array(&c_len);
    assert(c_len == c_array.size() && "C-array length mismatch");
    
    // SSA
    size_t ssa_len = 0;
    uint32_t ssa_stride = 0;
    const uint32_t* ssa_read = reader.get_ssa(&ssa_len, &ssa_stride);
    assert(ssa_stride == stride && "SSA stride mismatch");
    assert(ssa_len == ssa_samples.size() && "SSA length mismatch");
    
    // Wavelet
    size_t wavelet_size = 0;
    const uint8_t* wavelet_ptr = reader.get_wavelet(&wavelet_size);
    assert(wavelet_ptr != nullptr && "Wavelet should not be null");
  }
  
  cleanup_test_file();
  std::cout << "  ✓ Full index roundtrip passed\n";
}

// ──────────────────────────────────────────────────────────────
// Main test driver
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "=== Running serialization_tests ===\n";

  test_header_roundtrip();
  test_text_roundtrip();
  test_bwt_roundtrip();
  test_c_array_roundtrip();
  test_ssa_roundtrip();
  test_wavelet_roundtrip();
  test_veb_layout_roundtrip();
  test_full_index();

  std::cout << "=== All serialization_tests passed! ===\n";
  return 0;
}

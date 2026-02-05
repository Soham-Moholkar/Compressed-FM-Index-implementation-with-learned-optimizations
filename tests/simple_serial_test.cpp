/**
 * simple_serial_test.cpp — Minimal serialization test to debug hangs.
 */

#include "../src/serialization/serialization.hpp"
#include <iostream>

using namespace cs;

int main() {
  std::cout << "Test 1: Write simple file...\n";
  
  try {
    // Write
    {
      std::cout << "  Creating writer...\n";
      IndexWriter writer("test_simple.csidx");
      std::cout << "  Writing header...\n";
      writer.write_header(0, 10);
      std::cout << "  Finalizing...\n";
      writer.finalize();
      std::cout << "  Write complete!\n";
    }
    
    std::cout << "\nTest 2: Read simple file...\n";
    
    // Read
    {
      std::cout << "  Creating reader...\n";
      IndexReader reader("test_simple.csidx");
      std::cout << "  Getting header...\n";
      const IndexHeader* hdr = reader.header();
      std::cout << "  Header version: " << hdr->version << "\n";
      std::cout << "  Header text_len: " << hdr->text_len << "\n";
      std::cout << "  Read complete!\n";
    }
    
    std::cout << "\n✓ All tests passed!\n";
    
    // Cleanup
    std::remove("test_simple.csidx");
    
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
  
  return 0;
}

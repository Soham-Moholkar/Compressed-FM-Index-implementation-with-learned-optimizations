#include "../src/api/fm_index.hpp"
#include <iostream>

using namespace cs;

int main() {
  // Use terminator to make BWT well-defined
  std::string text = "banana$";
  BuildParams params;
  params.ssa_stride = 2;  // Use small stride for small text.
  FMIndex idx = FMIndex::build_from_text(text, params);
  
  std::cout << "Text: " << text << "\n";
  std::cout << "Length: " << text.size() << "\n\n";
  
  // Test 'a'
  std::cout << "Pattern 'a': count=" << idx.count("a") << "\n";
  try {
    auto pos_a = idx.locate("a");
    std::cout << "  Positions: ";
    for (auto p : pos_a) std::cout << p << " ";
    std::cout << "\n\n";
  } catch (const std::exception& e) {
    std::cerr << "ERROR in locate('a'): " << e.what() << "\n";
    return 1;
  }
  
  // Test 'aa'
  std::cout << "Pattern 'aa': count=" << idx.count("aa") << "\n";
  auto pos_aa = idx.locate("aa");
  std::cout << "  Positions: ";
  for (auto p : pos_aa) std::cout << p << " ";
  std::cout << "\n\n";
  
  // Test 'aab'
  std::cout << "Pattern 'aab': count=" << idx.count("aab") << "\n";
  auto pos_aab = idx.locate("aab");
  std::cout << "  Positions: ";
  for (auto p : pos_aab) std::cout << p << " ";
  std::cout << "\n";
  
  return 0;
}

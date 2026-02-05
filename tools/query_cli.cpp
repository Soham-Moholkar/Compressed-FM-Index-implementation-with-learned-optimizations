#include <iostream>
#include "../src/api/fm_index.hpp"
#include "../src/util/io.hpp"

int main(int argc, char** argv){
  if (argc < 3){
    std::cerr << "usage: cs_query <input.txt> <pattern>\n";
    return 1;
  }
  auto text = cs::slurp(argv[1]);
  auto idx = cs::FMIndex::build_from_text(text, {});
  auto c = idx.count(argv[2]);
  auto pos = idx.locate(argv[2], 100);
  std::cout << "count=" << c << "\npositions: ";
  for (auto p: pos) std::cout << p << " ";
  std::cout << "\n";
  return 0;
}

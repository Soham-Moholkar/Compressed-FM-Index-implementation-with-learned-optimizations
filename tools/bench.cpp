#include <iostream>
#include <random>
#include "../src/api/fm_index.hpp"
#include "../src/util/io.hpp"
#include "../src/util/timer.hpp"

int main(int argc, char** argv){
  if (argc < 2){ std::cerr << "usage: cs_bench <input.txt>\n"; return 1; }
  auto text = cs::slurp(argv[1]);
  auto idx = cs::FMIndex::build_from_text(text, {});
  std::mt19937 rng(42);
  std::uniform_int_distribution<size_t> L(3, 12), P(0, text.size() ? text.size()-20 : 0);
  size_t iters = 2000, total = 0;
  {
    cs::ScopeTimer t("bench_count");
    for(size_t it=0; it<iters; ++it){
      size_t pos = P(rng), len = L(rng);
      if (pos + len > text.size()) len = text.size() - pos;
      auto q = std::string_view(text).substr(pos, len);
      total += idx.count(q);
    }
  }
  std::cerr << "agg=" << total << "\n";
  return 0;
}

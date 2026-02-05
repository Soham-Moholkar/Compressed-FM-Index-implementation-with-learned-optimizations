#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

namespace cs {
inline std::vector<uint32_t> build_sa_naive(const std::string& T){
  uint32_t n = (uint32_t)T.size();
  std::vector<uint32_t> sa(n);
  for(uint32_t i=0;i<n;++i) sa[i]=i;
  std::sort(sa.begin(), sa.end(), [&](uint32_t a, uint32_t b){
    return T.substr(a) < T.substr(b);
  });
  return sa;
}
} // namespace cs

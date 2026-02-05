#pragma once
#include <vector>
#include <cstdint>
#include <string>

namespace cs {
inline std::string build_bwt_from_sa(const std::string& T, const std::vector<uint32_t>& sa){
  uint32_t n = (uint32_t)T.size();
  std::string BWT; BWT.resize(n);
  for(uint32_t i=0;i<n;++i){
    uint32_t idx = sa[i];
    BWT[i] = (idx==0) ? T[n-1] : T[idx-1];
  }
  return BWT;
}
} // namespace cs

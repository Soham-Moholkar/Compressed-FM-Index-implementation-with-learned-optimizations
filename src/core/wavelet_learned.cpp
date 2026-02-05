#include "wavelet_learned.hpp"
#include <stdexcept>
#include <cstring>

namespace cs {

void WaveletLevel::build(const std::vector<uint8_t>& bits_linear,
                         const LearnedOccConfig& cfg){
  cfg_ = cfg;
  nbits_ = (uint32_t)bits_linear.size();
  std::vector<uint64_t> tmp((nbits_ + 63)/64, 0);
  for(uint32_t i=0;i<nbits_;++i) if (bits_linear[i]) tmp[i>>6] |= (1ULL << (i&63));
  // Note: VebLayout integration deferred to later in STEP 5
  bits_co_ = std::move(tmp);

  const uint32_t S = cfg_.coarse_stride_S, s = cfg_.micro_stride_s;
  uint32_t nBuckets = (nbits_ + S - 1) / S;
  std::vector<uint32_t> xs; xs.reserve(nBuckets+1);
  std::vector<uint32_t> ys; ys.reserve(nBuckets+1);

  for(uint32_t j=0;j<=nBuckets;++j){
    uint32_t pos = std::min(j*S, nbits_);
    uint32_t words = pos >> 6, bits = pos & 63, pc=0;
    for(uint32_t w=0; w<words; ++w) pc += popcount64(bits_co_[w]);
    if(bits) pc += popcount64(bits_co_[words] & ((1ULL<<bits)-1));
    xs.push_back(j*S); ys.push_back(pc);
  }
  pgm_ = PgmModel::fit(xs, ys, cfg_.pgm_eps);

  if (S && s && (S % s == 0)) {
    uint32_t cols = S / s;
    residuals_.assign(nBuckets * cols, 0);
    for(uint32_t j=0;j<nBuckets;++j){
      uint32_t base = j*S;
      int32_t prev = 0;
      for(uint32_t m=1; m<=cols; ++m){
        uint32_t pos = base + m*s;
        if (pos>nbits_) pos = nbits_;
        uint32_t words = pos >> 6, bits = pos & 63, pc=0;
        for(uint32_t w=0; w<words; ++w) pc += popcount64(bits_co_[w]);
        if(bits) pc += popcount64(bits_co_[words] & ((1ULL<<bits)-1));
        int32_t delta = (int32_t)pc - prev;
        residuals_[j*cols + (m-1)] = delta;
        prev = (int32_t)pc;
      }
    }
  }
}

uint32_t WaveletLevel::popcount_tail(uint32_t bitpos, uint32_t len) const {
  if (len==0) return 0;
  uint32_t startWord = bitpos >> 6;
  uint32_t offset    = bitpos & 63;
  uint64_t word = bits_co_[startWord];
  if (offset + len <= 64) {
    uint64_t mask = (len==64? ~0ULL : ((1ULL<<len)-1)) << offset;
    return popcount64(word & mask);
  } else {
    uint32_t first = 64 - offset, rest = len - first;
    uint32_t pc = popcount64(word & (~0ULL << offset));
    uint64_t word2 = bits_co_[startWord+1];
    uint64_t mask2 = (rest==64? ~0ULL : ((1ULL<<rest)-1));
    pc += popcount64(word2 & mask2);
    return pc;
  }
}
} // namespace cs

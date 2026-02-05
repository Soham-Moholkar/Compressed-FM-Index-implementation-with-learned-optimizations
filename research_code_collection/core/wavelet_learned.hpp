#pragma once
#include <vector>
#include <cstdint>
#include "../learned/pgm.hpp"
#include "../layout/veb.hpp"
#include "../util/bitops.hpp"

namespace cs {

struct LearnedOccConfig {
  uint32_t coarse_stride_S = 512;
  uint32_t micro_stride_s  = 64;
  uint32_t veb_leaf_bytes  = 64;
  double   pgm_eps         = 1.0;
};

class WaveletLevel {
public:
  void build(const std::vector<uint8_t>& bits_linear, const LearnedOccConfig& cfg);

  inline uint32_t rank1(uint32_t i) const {
    if (i==0) return 0;
    if (i>nbits_) i = nbits_;
    const uint32_t S = cfg_.coarse_stride_S, s = cfg_.micro_stride_s;
    uint32_t j = i / S, i0 = j * S;
    int32_t pred = pgm_.predict_prefix(i0);
    uint32_t off = i - i0, micro = off / s;
    int32_t delta = 0;
    if (!residuals_.empty()) {
      uint32_t cols = S / s;
      uint32_t idx = j * cols + (micro ? (micro-1) : 0);
      if (idx < residuals_.size()) delta = residuals_[idx];
    }
    uint32_t rem = off - micro*s;
    uint32_t tail = popcount_tail(i0 + micro*s, rem);
    int64_t res = (int64_t)pred + delta + tail;
    return res < 0 ? 0u : (uint32_t)res;
  }

  uint32_t nbits() const { return nbits_; }

private:
  uint32_t popcount_tail(uint32_t bitpos, uint32_t len) const;

  LearnedOccConfig cfg_{};
  uint32_t nbits_{0};
  std::vector<uint64_t> bits_co_;
  std::vector<int32_t>  residuals_;
  PgmModel pgm_;
  // Note: VebLayout will be integrated later in STEP 5
};
} // namespace cs

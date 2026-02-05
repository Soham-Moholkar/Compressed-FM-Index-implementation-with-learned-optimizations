#pragma once
/**
 * bitvector_learned.hpp — Learned BitVector with PGM + bounded-touch rank queries.
 *
 * Architecture:
 *   - Coarse samples every S bits: PGM models (position → rank)
 *   - Micro samples every s bits: residuals (correction deltas)
 *   - Tail popcounts: ≤R aligned 64-bit word pops (bounded touch)
 *
 * rank1(i) algorithm:
 *   1. Coarse prediction: p = PGM(⌊i/S⌋ * S)
 *   2. Micro correction: δ = residual[⌊i/s⌋]
 *   3. Tail popcount: t = popcount in [⌊i/s⌋ * s, i) (≤R words)
 *   4. Return p + δ + t
 *
 * Enabled via CS_USE_LEARNED_OCC=1 compile flag.
 */

#include "../../include/cs/config.hpp"
#include "../learned/pgm.hpp"
#include "../util/bitops.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace cs {

class BitvectorLearned {
public:
  BitvectorLearned() = default;

  /**
   * Build learned bitvector from unpacked bits.
   * 
   * @param bits Unpacked bitvector (0 or 1)
   * @param S Coarse stride for PGM samples (default: CS_COARSE_STRIDE_S)
   * @param s Micro stride for residuals (default: CS_MICRO_STRIDE_s)
   * @param pgm_eps PGM error tolerance (default: 1.0)
   */
  void build(const std::vector<uint8_t>& bits,
             uint32_t S = CS_COARSE_STRIDE_S,
             uint32_t s = CS_MICRO_STRIDE_s,
             double pgm_eps = 1.0);

  /**
   * Build from pre-packed words.
   */
  void build_from_words(const std::vector<uint64_t>& words, size_t nbits,
                        uint32_t S = CS_COARSE_STRIDE_S,
                        uint32_t s = CS_MICRO_STRIDE_s,
                        double pgm_eps = 1.0);

  /// Number of bits.
  inline size_t size() const { return nbits_; }

  /// Get bit at position i.
  inline uint8_t get(size_t i) const {
    if (i >= nbits_) return 0;
    const size_t word_idx = i / 64;
    const size_t bit_idx = i % 64;
    return (bits_[word_idx] >> bit_idx) & 1u;
  }

  /**
   * rank1(i) = number of 1-bits in [0, i) using learned index.
   * 
   * Bounded-touch guarantee: accesses ≤R words for tail popcount.
   */
  size_t rank1(size_t i) const;

  /**
   * rank0(i) = number of 0-bits in [0, i).
   */
  inline size_t rank0(size_t i) const {
    if (i > nbits_) i = nbits_;
    return i - rank1(i);
  }

  /// Total number of 1s (for testing).
  size_t count_ones() const;

private:
  size_t nbits_ = 0;                 ///< Logical bit count.
  uint32_t S_ = CS_COARSE_STRIDE_S;  ///< Coarse stride.
  uint32_t s_ = CS_MICRO_STRIDE_s;   ///< Micro stride.
  
  std::vector<uint64_t> bits_;       ///< Packed bitvector.
  PgmModel pgm_;                     ///< Learned model for coarse samples.
  std::vector<int32_t> residuals_;   ///< Micro corrections (one per micro-block).
};

} // namespace cs

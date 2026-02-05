#pragma once
/**
 * bitvector.hpp — Two-level sampled rank/select data structure.
 *
 * Memory layout:
 *   bits_[w] : packed 64-bit words storing the raw bitvector
 *   super_[j]: absolute rank1 at position j*SUPER_BLOCK_SIZE (uint32_t)
 *   blocks_[k]: relative rank1 within super-block (uint16_t, every SUB_BLOCK_SIZE)
 *
 * rank1(i) returns the number of 1-bits in [0, i) — half-open interval.
 * rank0(i) = i - rank1(i).
 * 
 * Uses existing popcount utilities from util/bitops.hpp for fast word counts.
 */

#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include "../../include/cs/config.hpp"
#include "../util/bitops.hpp"

namespace cs {

class BitVector {
public:
  BitVector() = default;

  /**
   * Build from a vector of bits (0 or 1).
   * 
   * @param bits Linear array of 0/1 values (need not be packed).
   */
  void build(const std::vector<uint8_t>& bits);

  /**
   * Build from pre-packed 64-bit words.
   * 
   * @param words Packed bitvector words (LSB = bit 0).
   * @param nbits Logical number of bits.
   */
  void build_from_words(const std::vector<uint64_t>& words, size_t nbits);

  /// Number of bits in the bitvector.
  inline size_t size() const { return nbits_; }

  /// Get bit at position i (0-indexed). Returns 0 or 1.
  inline uint8_t get(size_t i) const {
    if (i >= nbits_) return 0;
    const size_t word_idx = i / 64;
    const size_t bit_idx  = i % 64;
    return (bits_[word_idx] >> bit_idx) & 1u;
  }

  /**
   * rank1(i) = number of 1-bits in [0, i) — half-open interval.
   * 
   * Implementation:
   *   1) Find super-block j = i / SUPER_BLOCK_SIZE → super_[j] = rank1(j * SUPER).
   *   2) Find sub-block k within super-block → blocks_[offset + k] = relative rank within super.
   *   3) Popcount remaining bits within the final 64-bit word (mask low bits).
   * 
   * Edge cases:
   *   - rank1(0) = 0 by definition.
   *   - rank1(i >= nbits_) = total number of 1s.
   */
  size_t rank1(size_t i) const;

  /**
   * rank0(i) = number of 0-bits in [0, i).
   */
  inline size_t rank0(size_t i) const {
    if (i > nbits_) i = nbits_;
    return i - rank1(i);
  }

  /**
   * (Optional) select1(k) = position of the k-th 1-bit (1-indexed k).
   * Not implemented in STEP 1; left for future extension.
   */
  // size_t select1(size_t k) const;

  /// For debugging: count all 1s (should equal rank1(size())).
  size_t count_ones() const;

  // ─────────────────────────────────────────────────────────
  // Public accessors for internal data (for vEB layout)
  // ─────────────────────────────────────────────────────────
  
  const std::vector<uint64_t>& bits() const { return bits_; }
  const std::vector<uint32_t>& super_blocks() const { return super_; }
  const std::vector<uint16_t>& sub_blocks() const { return blocks_; }

private:
  size_t nbits_ = 0;                  ///< Logical bit count.
  std::vector<uint64_t> bits_;        ///< Packed bitvector (64-bit words).
  std::vector<uint32_t> super_;       ///< Absolute rank1 every SUPER_BLOCK_SIZE bits.
  std::vector<uint16_t> blocks_;      ///< Relative rank1 every SUB_BLOCK_SIZE within super-block.
};

} // namespace cs

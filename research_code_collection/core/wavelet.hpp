#pragma once
/**
 * wavelet.hpp — Binary wavelet tree for byte alphabet (0..255).
 *
 * Structure:
 *   - 8 levels (one per bit position in a byte)
 *   - Each level uses BitVector for O(1) rank queries
 *   - Level 0 = MSB (bit 7), Level 7 = LSB (bit 0)
 *
 * API:
 *   - rank(c, i): Count of symbol c in BWT[0..i)
 *   - access(i): Return BWT[i]
 *
 * Construction:
 *   Given BWT string, build all 8 levels by partitioning on each bit.
 */

#include "bitvector.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <array>

namespace cs {

class WaveletTree {
public:
  WaveletTree() = default;

  /**
   * Build wavelet tree from BWT string.
   * 
   * @param bwt The BWT-transformed text (byte alphabet).
   */
  void build(const std::vector<uint8_t>& bwt);

  /**
   * rank(c, i) = number of occurrences of symbol c in bwt[0..i).
   * 
   * Half-open interval [0, i) — critical for FM-index.
   * Returns 0 if i==0 or c not present.
   */
  size_t rank(uint8_t c, size_t i) const;

  /**
   * access(i) = bwt[i] — retrieve symbol at position i.
   * 
   * Implemented by descending the tree following bit values.
   */
  uint8_t access(size_t i) const;

  /// Number of symbols in the BWT.
  size_t size() const { return n_; }

private:
  size_t n_ = 0;                          ///< Length of BWT.
  std::array<BitVector, 8> levels_;       ///< One BitVector per bit (MSB to LSB).
};

} // namespace cs

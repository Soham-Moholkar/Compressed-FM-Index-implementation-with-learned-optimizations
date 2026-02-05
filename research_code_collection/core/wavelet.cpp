/**
 * wavelet.cpp — Binary wavelet tree implementation.
 */

#include "wavelet.hpp"
#include <cassert>

namespace cs {

// ──────────────────────────────────────────────────────────────
// build: Construct 8-level binary wavelet tree
// ──────────────────────────────────────────────────────────────

void WaveletTree::build(const std::vector<uint8_t>& bwt) {
  n_ = bwt.size();
  if (n_ == 0) return;

  // Build levels from MSB (bit 7) to LSB (bit 0).
  // At each level, bit=0 goes left, bit=1 goes right.
  
  std::vector<uint8_t> current = bwt;  // Symbols at current level.

  for (int bit = 7; bit >= 0; --bit) {
    const int level = 7 - bit;  // Level 0 = MSB (bit 7), Level 7 = LSB (bit 0).
    
    // Build bitvector for this level: 1 if bit is set, 0 otherwise.
    std::vector<uint8_t> bitvec(current.size());
    std::vector<uint8_t> left, right;
    left.reserve(current.size());
    right.reserve(current.size());

    for (size_t i = 0; i < current.size(); ++i) {
      const uint8_t sym = current[i];
      const uint8_t bit_val = (sym >> bit) & 1;
      bitvec[i] = bit_val;

      if (bit_val == 0) {
        left.push_back(sym);
      } else {
        right.push_back(sym);
      }
    }

    // Build BitVector for this level.
    levels_[level].build(bitvec);

    // For next level, concatenate left and right partitions.
    if (bit > 0) {  // Not the last level.
      current = std::move(left);
      current.insert(current.end(), right.begin(), right.end());
    }
  }
}

// ──────────────────────────────────────────────────────────────
// rank(c, i): Count of symbol c in [0, i)
// ──────────────────────────────────────────────────────────────

size_t WaveletTree::rank(uint8_t c, size_t i) const {
  if (i == 0 || i > n_) return 0;
  if (i > n_) i = n_;

  size_t start = 0;  // Start of current range.
  size_t end = i;    // End of current range (half-open).

  // Descend from MSB (level 0) to LSB (level 7).
  for (int level = 0; level < 8; ++level) {
    const int bit = 7 - level;  // Which bit we're looking at.
    const uint8_t bit_val = (c >> bit) & 1;

    const BitVector& bv = levels_[level];

    if (bit_val == 0) {
      // Go left: count 0s in [start, end).
      const size_t rank0_start = start - bv.rank1(start);
      const size_t rank0_end = end - bv.rank1(end);
      start = rank0_start;
      end = rank0_end;
    } else {
      // Go right: count 1s in [start, end).
      const size_t rank1_start = bv.rank1(start);
      const size_t rank1_end = bv.rank1(end);
      
      // Right partition starts after all 0s in this level.
      const size_t zeros_total = bv.size() - bv.rank1(bv.size());
      start = zeros_total + rank1_start;
      end = zeros_total + rank1_end;
    }

    // If range becomes empty, symbol c doesn't appear in [0, i).
    if (start >= end) return 0;
  }

  // After descending all 8 levels, end - start = count of c in [0, i).
  return end - start;
}

// ──────────────────────────────────────────────────────────────
// access(i): Retrieve symbol at position i
// ──────────────────────────────────────────────────────────────

uint8_t WaveletTree::access(size_t i) const {
  assert(i < n_);
  if (i >= n_) return 0;

  uint8_t symbol = 0;
  size_t pos = i;

  // Descend from MSB (level 0) to LSB (level 7), reconstructing symbol.
  for (int level = 0; level < 8; ++level) {
    const int bit = 7 - level;
    const BitVector& bv = levels_[level];

    const uint8_t bit_val = bv.get(pos);
    symbol |= (bit_val << bit);

    if (bit_val == 0) {
      // Go left: position among 0s.
      pos = pos - bv.rank1(pos);
    } else {
      // Go right: position among 1s.
      const size_t zeros_total = bv.size() - bv.rank1(bv.size());
      pos = zeros_total + bv.rank1(pos);
    }
  }

  return symbol;
}

} // namespace cs

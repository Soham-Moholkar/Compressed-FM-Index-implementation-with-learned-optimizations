/**
 * bitvector.cpp — Implementation of two-level sampled rank/select.
 */

#include "bitvector.hpp"
#include <cassert>

namespace cs {

// ──────────────────────────────────────────────────────────────
// build: construct from unpacked bits (0 or 1)
// ──────────────────────────────────────────────────────────────

void BitVector::build(const std::vector<uint8_t>& bits) {
  nbits_ = bits.size();
  if (nbits_ == 0) {
    bits_.clear();
    super_.clear();
    blocks_.clear();
    return;
  }

  // 1) Pack bits into 64-bit words (LSB = bit 0).
  const size_t nwords = (nbits_ + 63) / 64;
  bits_.assign(nwords, 0);
  for (size_t i = 0; i < nbits_; ++i) {
    if (bits[i]) {
      const size_t word_idx = i / 64;
      const size_t bit_idx  = i % 64;
      bits_[word_idx] |= (1ULL << bit_idx);
    }
  }

  // 2) Build two-level rank index.
  constexpr size_t SUPER = CS_SUPER_BLOCK_SIZE;
  constexpr size_t SUB   = CS_SUB_BLOCK_SIZE;
  constexpr size_t SUBS_PER_SUPER = SUPER / SUB;

  const size_t num_supers = (nbits_ + SUPER - 1) / SUPER;
  super_.reserve(num_supers);

  // One sub-block entry per SUB bits (including the first sub within each super).
  const size_t num_subs = (nbits_ + SUB - 1) / SUB;
  blocks_.reserve(num_subs);

  size_t running_rank = 0;  // Absolute rank across the entire bitvector.

  for (size_t super_idx = 0; super_idx < num_supers; ++super_idx) {
    // Store absolute rank at the start of this super-block.
    super_.push_back(static_cast<uint32_t>(running_rank));

    const size_t super_start_bit = super_idx * SUPER;
    const size_t super_end_bit   = std::min(super_start_bit + SUPER, nbits_);
    size_t local_rank = 0;  // Rank within this super-block.

    // Process each sub-block within this super-block.
    for (size_t sub_offset = 0; sub_offset < SUBS_PER_SUPER; ++sub_offset) {
      const size_t sub_start_bit = super_start_bit + sub_offset * SUB;
      if (sub_start_bit >= nbits_) break;

      // Store relative rank at the start of this sub-block.
      blocks_.push_back(static_cast<uint16_t>(local_rank));

      const size_t sub_end_bit = std::min(sub_start_bit + SUB, super_end_bit);
      const size_t sub_len = sub_end_bit - sub_start_bit;

      // Popcount this sub-block: iterate over its 64-bit words.
      const size_t word_start = sub_start_bit / 64;
      const size_t word_end   = (sub_end_bit + 63) / 64;

      for (size_t w = word_start; w < word_end; ++w) {
        uint64_t word = bits_[w];
        const size_t word_bit_start = w * 64;
        const size_t word_bit_end   = word_bit_start + 64;

        // Mask out bits outside [sub_start_bit, sub_end_bit).
        if (word_bit_start < sub_start_bit) {
          const size_t trim_low = sub_start_bit - word_bit_start;
          word &= (~0ULL << trim_low);
        }
        if (word_bit_end > sub_end_bit) {
          const size_t trim_high = word_bit_end - sub_end_bit;
          word &= (~0ULL >> trim_high);
        }

        const uint32_t pop = popcount64(word);
        local_rank += pop;
        running_rank += pop;
      }
    }
  }
}

// ──────────────────────────────────────────────────────────────
// build_from_words: construct from pre-packed 64-bit words
// ──────────────────────────────────────────────────────────────

void BitVector::build_from_words(const std::vector<uint64_t>& words, size_t nbits) {
  nbits_ = nbits;
  bits_ = words;

  // Ensure bits_ has enough words.
  const size_t required_words = (nbits_ + 63) / 64;
  if (bits_.size() < required_words) {
    bits_.resize(required_words, 0);
  }

  // Build rank index (same logic as build()).
  constexpr size_t SUPER = CS_SUPER_BLOCK_SIZE;
  constexpr size_t SUB   = CS_SUB_BLOCK_SIZE;
  constexpr size_t SUBS_PER_SUPER = SUPER / SUB;

  const size_t num_supers = (nbits_ + SUPER - 1) / SUPER;
  super_.reserve(num_supers);

  const size_t num_subs = (nbits_ + SUB - 1) / SUB;
  blocks_.reserve(num_subs);

  size_t running_rank = 0;

  for (size_t super_idx = 0; super_idx < num_supers; ++super_idx) {
    super_.push_back(static_cast<uint32_t>(running_rank));

    const size_t super_start_bit = super_idx * SUPER;
    const size_t super_end_bit   = std::min(super_start_bit + SUPER, nbits_);
    size_t local_rank = 0;

    for (size_t sub_offset = 0; sub_offset < SUBS_PER_SUPER; ++sub_offset) {
      const size_t sub_start_bit = super_start_bit + sub_offset * SUB;
      if (sub_start_bit >= nbits_) break;

      blocks_.push_back(static_cast<uint16_t>(local_rank));

      const size_t sub_end_bit = std::min(sub_start_bit + SUB, super_end_bit);

      const size_t word_start = sub_start_bit / 64;
      const size_t word_end   = (sub_end_bit + 63) / 64;

      for (size_t w = word_start; w < word_end; ++w) {
        uint64_t word = bits_[w];
        const size_t word_bit_start = w * 64;
        const size_t word_bit_end   = word_bit_start + 64;

        if (word_bit_start < sub_start_bit) {
          const size_t trim_low = sub_start_bit - word_bit_start;
          word &= (~0ULL << trim_low);
        }
        if (word_bit_end > sub_end_bit) {
          const size_t trim_high = word_bit_end - sub_end_bit;
          word &= (~0ULL >> trim_high);
        }

        const uint32_t pop = popcount64(word);
        local_rank += pop;
        running_rank += pop;
      }
    }
  }
}

// ──────────────────────────────────────────────────────────────
// rank1(i): number of 1-bits in [0, i)
// ──────────────────────────────────────────────────────────────

size_t BitVector::rank1(size_t i) const {
  if (i == 0) return 0;
  if (i >= nbits_) {
    // For i >= nbits_, return the total count.
    return count_ones();
  }

  constexpr size_t SUPER = CS_SUPER_BLOCK_SIZE;
  constexpr size_t SUB   = CS_SUB_BLOCK_SIZE;

  // 1) Find super-block.
  const size_t super_idx = i / SUPER;
  assert(super_idx < super_.size());
  size_t rank = super_[super_idx];

  const size_t super_start = super_idx * SUPER;
  const size_t offset_in_super = i - super_start;

  // If i is exactly at a super-block boundary, return the super-block rank.
  if (offset_in_super == 0) {
    return rank;
  }

  // 2) Find sub-block within super-block.
  const size_t sub_offset = offset_in_super / SUB;  // Which sub within this super?
  const size_t blocks_base = super_idx * (SUPER / SUB);
  const size_t block_idx = blocks_base + sub_offset;

  if (block_idx < blocks_.size()) {
    rank += blocks_[block_idx];
  }

  const size_t sub_start = super_start + sub_offset * SUB;

  // If i is exactly at a sub-block boundary, no additional popcounting needed.
  if (i == sub_start) {
    return rank;
  }

  // 3) Popcount the remaining bits in the final word(s) from sub_start to i.
  const size_t word_start = sub_start / 64;
  const size_t word_end   = i == 0 ? 0 : ((i - 1) / 64);

  for (size_t w = word_start; w <= word_end && w < bits_.size(); ++w) {
    uint64_t word = bits_[w];
    const size_t word_bit_start = w * 64;

    // Mask out bits before sub_start.
    if (word_bit_start < sub_start) {
      const size_t trim_low = sub_start - word_bit_start;
      word &= (~0ULL << trim_low);
    }
    // Mask out bits >= i (keep only bits [word_bit_start, i)).
    const size_t word_bit_end = word_bit_start + 64;
    if (word_bit_end > i) {
      const size_t keep_bits = i - word_bit_start;
      if (keep_bits < 64) {
        word &= ((1ULL << keep_bits) - 1);
      }
    }

    rank += popcount64(word);
  }

  return rank;
}

// ──────────────────────────────────────────────────────────────
// count_ones: for debugging/testing
// ──────────────────────────────────────────────────────────────

size_t BitVector::count_ones() const {
  size_t total = 0;
  for (size_t w = 0; w < bits_.size(); ++w) {
    uint64_t word = bits_[w];
    // Mask out bits beyond nbits_ in the last word.
    if ((w + 1) * 64 > nbits_) {
      const size_t valid_bits = nbits_ - w * 64;
      word &= ((1ULL << valid_bits) - 1);
    }
    total += popcount64(word);
  }
  return total;
}

} // namespace cs

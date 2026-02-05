/**
 * bitvector_learned.cpp — Implementation of learned bitvector with bounded-touch rank.
 */

#include "bitvector_learned.hpp"
#include <algorithm>
#include <cassert>

namespace cs {

// ──────────────────────────────────────────────────────────────
// build: Construct learned index from unpacked bits
// ──────────────────────────────────────────────────────────────

void BitvectorLearned::build(const std::vector<uint8_t>& bits,
                             uint32_t S, uint32_t s, double pgm_eps) {
  nbits_ = bits.size();
  S_ = S;
  s_ = s;

  if (nbits_ == 0) {
    bits_.clear();
    residuals_.clear();
    pgm_ = PgmModel();
    return;
  }

  // 1) Pack bits into 64-bit words.
  const size_t nwords = (nbits_ + 63) / 64;
  bits_.assign(nwords, 0);
  for (size_t i = 0; i < nbits_; ++i) {
    if (bits[i]) {
      bits_[i / 64] |= (1ULL << (i % 64));
    }
  }

  // 2) Build coarse samples every S bits for PGM.
  const size_t num_coarse = (nbits_ + S - 1) / S + 1;  // Include endpoint.
  std::vector<uint32_t> coarse_x, coarse_y;
  coarse_x.reserve(num_coarse);
  coarse_y.reserve(num_coarse);

  size_t running_rank = 0;
  for (size_t j = 0; j <= (nbits_ + S - 1) / S; ++j) {
    const size_t pos = std::min(j * S, nbits_);
    coarse_x.push_back(static_cast<uint32_t>(pos));
    coarse_y.push_back(static_cast<uint32_t>(running_rank));

    // Accumulate rank for next coarse sample.
    if (pos < nbits_) {
      const size_t next_pos = std::min((j + 1) * S, nbits_);
      for (size_t k = pos; k < next_pos; ++k) {
        if (bits[k]) ++running_rank;
      }
    }
  }

  // 3) Fit PGM model to coarse samples.
  pgm_ = PgmModel::fit(coarse_x, coarse_y, pgm_eps);

  // 4) Build micro residuals every s bits.
  if (s > 0 && S % s == 0) {
    const size_t mics_per_coarse = S / s;
    const size_t num_coarse_blocks = (nbits_ + S - 1) / S;
    residuals_.assign(num_coarse_blocks * mics_per_coarse, 0);

    running_rank = 0;
    for (size_t j = 0; j < num_coarse_blocks; ++j) {
      const size_t coarse_start = j * S;
      const int32_t pgm_pred = pgm_.predict(static_cast<uint32_t>(coarse_start));

      // Accumulate rank within this coarse block.
      size_t local_rank = 0;
      for (size_t m = 0; m < mics_per_coarse; ++m) {
        const size_t mic_start = coarse_start + m * s;
        if (mic_start >= nbits_) break;

        // Residual = true_rank(mic_start) - pgm_pred.
        const int32_t residual = static_cast<int32_t>(running_rank + local_rank) - pgm_pred;
        residuals_[j * mics_per_coarse + m] = residual;

        // Accumulate rank for this micro-block.
        const size_t mic_end = std::min(mic_start + s, std::min(coarse_start + S, nbits_));
        for (size_t k = mic_start; k < mic_end; ++k) {
          if (bits[k]) ++local_rank;
        }
      }

      running_rank += local_rank;
    }
  }
}

// ──────────────────────────────────────────────────────────────
// build_from_words: Construct from pre-packed words
// ──────────────────────────────────────────────────────────────

void BitvectorLearned::build_from_words(const std::vector<uint64_t>& words, size_t nbits,
                                        uint32_t S, uint32_t s, double pgm_eps) {
  // Unpack to build (inefficient but simple for now).
  std::vector<uint8_t> bits(nbits);
  for (size_t i = 0; i < nbits; ++i) {
    const size_t word_idx = i / 64;
    const size_t bit_idx = i % 64;
    bits[i] = (word_idx < words.size() && ((words[word_idx] >> bit_idx) & 1)) ? 1 : 0;
  }
  build(bits, S, s, pgm_eps);
}

// ──────────────────────────────────────────────────────────────
// rank1: Learned rank query with bounded touch
// ──────────────────────────────────────────────────────────────

size_t BitvectorLearned::rank1(size_t i) const {
  if (i == 0) return 0;
  if (i >= nbits_) {
    return count_ones();
  }

  const size_t S = S_;
  const size_t s = s_;

  // 1) Coarse prediction from PGM.
  const size_t coarse_idx = i / S;
  const size_t coarse_pos = coarse_idx * S;
  int32_t pred = pgm_.predict(static_cast<uint32_t>(coarse_pos));

  // 2) Micro correction from residuals.
  const size_t offset_in_coarse = i - coarse_pos;
  const size_t micro_idx = offset_in_coarse / s;
  
  int32_t correction = 0;
  if (!residuals_.empty() && s > 0) {
    const size_t mics_per_coarse = S / s;
    const size_t residual_idx = coarse_idx * mics_per_coarse + micro_idx;
    if (residual_idx < residuals_.size()) {
      correction = residuals_[residual_idx];
    }
  }

  // 3) Tail popcount: from micro-block start to i.
  const size_t micro_start = coarse_pos + micro_idx * s;
  const size_t tail_len = i - micro_start;
  
  size_t tail_pop = 0;
  if (tail_len > 0) {
    // Bounded touch: iterate over ≤R words.
    const size_t word_start = micro_start / 64;
    const size_t word_end = (i - 1) / 64;
    const size_t num_words = word_end - word_start + 1;
    
    // Safety: limit to CS_MAX_TAIL_POPCOUNTS_R words.
    const size_t max_words = CS_MAX_TAIL_POPCOUNTS_R;
    if (num_words <= max_words) {
      for (size_t w = word_start; w <= word_end && w < bits_.size(); ++w) {
        uint64_t word = bits_[w];
        const size_t word_bit_start = w * 64;

        // Mask out bits before micro_start.
        if (word_bit_start < micro_start) {
          const size_t trim_low = micro_start - word_bit_start;
          word &= (~0ULL << trim_low);
        }

        // Mask out bits >= i.
        const size_t word_bit_end = word_bit_start + 64;
        if (word_bit_end > i) {
          const size_t keep_bits = i - word_bit_start;
          if (keep_bits < 64) {
            word &= ((1ULL << keep_bits) - 1);
          }
        }

        tail_pop += popcount64(word);
      }
    } else {
      // Exceeded bounded touch: fall back to scanning all words (rare case).
      for (size_t w = word_start; w <= word_end && w < bits_.size(); ++w) {
        uint64_t word = bits_[w];
        const size_t word_bit_start = w * 64;

        if (word_bit_start < micro_start) {
          const size_t trim_low = micro_start - word_bit_start;
          word &= (~0ULL << trim_low);
        }

        const size_t word_bit_end = word_bit_start + 64;
        if (word_bit_end > i) {
          const size_t keep_bits = i - word_bit_start;
          if (keep_bits < 64) {
            word &= ((1ULL << keep_bits) - 1);
          }
        }

        tail_pop += popcount64(word);
      }
    }
  }

  // 4) Combine: prediction + correction + tail.
  const int64_t result = static_cast<int64_t>(pred) + correction + static_cast<int64_t>(tail_pop);
  return result < 0 ? 0 : static_cast<size_t>(result);
}

// ──────────────────────────────────────────────────────────────
// count_ones: For testing/validation
// ──────────────────────────────────────────────────────────────

size_t BitvectorLearned::count_ones() const {
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

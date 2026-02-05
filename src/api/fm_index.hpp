#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../core/wavelet.hpp"
#include "../core/wavelet_learned.hpp"
#include "../core/ssa.hpp"

namespace cs {

struct BuildParams {
  uint32_t S = 512, s = 64, ssa_stride = 32;
  double eps = 1.0;
};
struct IndexMeta { uint64_t n = 0; uint32_t sigma = 256; };

class FMIndex {
public:
  static FMIndex build_from_text(const std::string& text, const BuildParams& p);
  static FMIndex open_directory(const std::string& dir); // TODO: on-disk format

  /**
   * count(pattern) — Number of occurrences of pattern in the indexed text.
   * Uses FM backward search with wavelet tree rank queries.
   */
  uint64_t count(std::string_view pattern) const;

  /**
   * locate(pattern, limit) — Positions where pattern occurs (up to limit).
   * Uses FM backward search + SSA to recover text positions.
   */
  std::vector<uint64_t> locate(std::string_view pattern, size_t limit=100000) const;

  /**
   * extract(pos, len) — Extract substring from indexed text.
   */
  std::string extract(uint64_t pos, uint64_t len) const;

private:
  IndexMeta meta_;
  std::string text_;                    // Original text (for extract/naive fallback).
  std::string bwt_;                     // BWT string (for locate via LF).
  std::vector<uint32_t> sa_;            // Full SA (temp, for SSA construction).
  std::vector<uint32_t> C_;             // Cumulative counts (byte alphabet).
  WaveletTree wavelet_;                 // Binary wavelet tree for BWT.
  SSA ssa_;                             // Sampled suffix array.
  
  // Legacy learned wavelet (kept for compatibility).
  std::vector<WaveletLevel> levels_;

  /**
   * occ(c, i) — Occurrences of symbol c in BWT[0..i).
   * Delegates to wavelet tree.
   */
  inline uint64_t occ(uint8_t c, uint64_t i) const {
    return wavelet_.rank(c, i);
  }

  /**
   * LF(i) — Last-to-First mapping: LF(i) = C[BWT[i]] + occ(BWT[i], i).
   */
  inline uint64_t LF(uint64_t i) const {
    if (i >= bwt_.size()) return 0;
    const uint8_t c = static_cast<uint8_t>(bwt_[i]);
    return C_[c] + occ(c, i);
  }
};
} // namespace cs

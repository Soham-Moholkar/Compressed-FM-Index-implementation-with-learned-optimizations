#include "fm_index.hpp"
#include "../core/sais.hpp"
#include "../core/bwt.hpp"
#include "../util/timer.hpp"
#include <array>
#include <algorithm>
#include <stdexcept>
#include <filesystem>

namespace cs {

// ──────────────────────────────────────────────────────────────
// build_from_text: Construct FM-index from input text
// ──────────────────────────────────────────────────────────────

FMIndex FMIndex::build_from_text(const std::string& text, const BuildParams& p) {
  FMIndex idx;
  idx.text_ = text;
  idx.meta_.n = text.size();

  // NOTE: For correct FM-index operation, text should include a unique terminator
  // (e.g., '$' or '\0') that is lexicographically smaller than all other characters.
  // This ensures the BWT is well-defined and unambiguous.

  // 1) Build suffix array (naive O(n^2 log n) for now).
  ScopeTimer t1("build_sa_naive");
  idx.sa_ = build_sa_naive(idx.text_);
  (void)t1;

  // 2) Build BWT from SA.
  ScopeTimer t2("build_bwt");
  idx.bwt_ = build_bwt_from_sa(text, idx.sa_);
  (void)t2;

  // 3) Build C array (cumulative character counts).
  idx.C_.assign(257, 0u);
  std::array<uint32_t, 256> freq{};
  freq.fill(0);
  for (unsigned char ch : idx.bwt_) {
    freq[ch]++;
  }
  uint32_t cum = 0;
  for (int c = 0; c < 256; ++c) {
    idx.C_[c] = cum;
    cum += freq[c];
  }
  idx.C_[256] = cum;

  // 4) Build binary wavelet tree over BWT.
  ScopeTimer t3("build_wavelet");
  std::vector<uint8_t> bwt_bytes(idx.bwt_.begin(), idx.bwt_.end());
  idx.wavelet_.build(bwt_bytes);
  (void)t3;

  // 5) Build sampled suffix array (SSA).
  // Sample SA values at BWT positions that are multiples of stride.
  ScopeTimer t4("build_ssa");
  idx.ssa_.stride = p.ssa_stride;
  const size_t num_samples = (idx.sa_.size() + p.ssa_stride - 1) / p.ssa_stride;
  idx.ssa_.samples.resize(num_samples);
  for (size_t i = 0; i < idx.sa_.size(); ++i) {
    if (i % p.ssa_stride == 0) {
      idx.ssa_.samples[i / p.ssa_stride] = idx.sa_[i];
    }
  }
  (void)t4;

  return idx;
}

FMIndex FMIndex::open_directory(const std::string&) {
  throw std::runtime_error("on-disk open not implemented yet");
}

// ──────────────────────────────────────────────────────────────
// count: FM backward search for pattern occurrences
// ──────────────────────────────────────────────────────────────

uint64_t FMIndex::count(std::string_view pattern) const {
  if (pattern.empty()) return meta_.n;
  if (meta_.n == 0) return 0;

  // FM backward search: start with full BWT range [0, n).
  uint64_t sp = 0;
  uint64_t ep = meta_.n;

  // Process pattern from right to left.
  for (auto it = pattern.rbegin(); it != pattern.rend(); ++it) {
    const uint8_t c = static_cast<uint8_t>(*it);
    
    // Update range: sp' = C[c] + occ(c, sp), ep' = C[c] + occ(c, ep).
    sp = C_[c] + occ(c, sp);
    ep = C_[c] + occ(c, ep);

    // If range becomes empty, pattern doesn't occur.
    if (sp >= ep) return 0;
  }

  // Number of occurrences = size of final range.
  return ep - sp;
}

// ──────────────────────────────────────────────────────────────
// locate: Find positions of pattern occurrences
// ──────────────────────────────────────────────────────────────

std::vector<uint64_t> FMIndex::locate(std::string_view pattern, size_t limit) const {
  std::vector<uint64_t> positions;
  if (pattern.empty() || meta_.n == 0) return positions;

  // 1) FM backward search to find range [sp, ep).
  uint64_t sp = 0;
  uint64_t ep = meta_.n;

  for (auto it = pattern.rbegin(); it != pattern.rend(); ++it) {
    const uint8_t c = static_cast<uint8_t>(*it);
    sp = C_[c] + occ(c, sp);
    ep = C_[c] + occ(c, ep);
    if (sp >= ep) return positions;
  }

  // 2) For each position in [sp, ep), recover text position via SSA + LF.
  positions.reserve(std::min<size_t>(ep - sp, limit));

  for (uint64_t i = sp; i < ep && positions.size() < limit; ++i) {
    uint64_t bwt_pos = i;
    uint64_t steps = 0;

    // Walk backwards via LF until we hit a sampled position.
    while (bwt_pos % ssa_.stride != 0 && steps < meta_.n) {
      bwt_pos = LF(bwt_pos);
      ++steps;
    }

    // Safety check.
    if (steps >= meta_.n) {
      throw std::runtime_error("locate: LF walk exceeded text length");
    }

    // Now bwt_pos is sampled: SA[bwt_pos] is stored.
    const uint64_t sample_idx = bwt_pos / ssa_.stride;
    if (sample_idx >= ssa_.samples.size()) {
      throw std::runtime_error("locate: SSA sample index out of range: idx=" + 
                               std::to_string(sample_idx) + ", size=" + 
                               std::to_string(ssa_.samples.size()));
    }
    const uint64_t sa_val = ssa_.samples[sample_idx];

    // LF-mapping walks backwards in the BWT, which corresponds to prepending characters.
    // If SA[sampled_pos] = k, and we walked 'steps' backwards via LF,
    // then we're looking at the suffix starting at position (k + steps) % n.
    const uint64_t text_pos = (sa_val + steps) % meta_.n;
    positions.push_back(text_pos);
  }

  return positions;
}

// ──────────────────────────────────────────────────────────────
// extract: Retrieve substring from original text
// ──────────────────────────────────────────────────────────────

std::string FMIndex::extract(uint64_t p, uint64_t len) const {
  if (p >= text_.size()) return {};
  len = std::min<uint64_t>(len, text_.size() - p);
  return text_.substr(p, len);
}

} // namespace cs

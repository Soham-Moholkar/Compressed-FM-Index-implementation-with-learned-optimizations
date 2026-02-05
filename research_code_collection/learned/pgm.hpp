#pragma once
/**
 * pgm.hpp — Simplified Piecewise Geometric Model (PGM) for learned indexing.
 *
 * Fits a piecewise linear approximation to (x, y) data points.
 * For simplicity, uses a single-segment linear regression in this implementation.
 * Production version would use multi-segment greedy algorithm with epsilon error bound.
 */

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace cs {

struct PgmSeg {
  float a, b;           // Line parameters: y = a*x + b
  uint32_t lo_idx;      // Start index in data
  uint32_t hi_idx;      // End index in data (exclusive)
};

struct PgmModel {
  std::vector<PgmSeg> segs;

  /**
   * Fit PGM model to data points (xs[i], ys[i]).
   * 
   * Simplified implementation: uses single-segment linear regression.
   * Full PGM would use greedy segmentation with epsilon error bound.
   * 
   * @param xs X coordinates (sorted, typically positions/strides)
   * @param ys Y coordinates (typically cumulative counts/ranks)
   * @param eps Error tolerance (not used in this simplified version)
   * @return Fitted PGM model
   */
  static PgmModel fit(const std::vector<uint32_t>& xs, const std::vector<uint32_t>& ys, double eps) {
    (void)eps;  // Reserved for full PGM implementation
    PgmModel M;
    
    if (xs.empty() || ys.empty() || xs.size() != ys.size()) {
      // Empty or invalid data: return identity model
      M.segs.push_back({0.0f, 0.0f, 0u, 1u});
      return M;
    }

    const size_t n = xs.size();
    
    // Simple linear regression: y = a*x + b
    // a = (n*Σxy - Σx*Σy) / (n*Σx² - (Σx)²)
    // b = (Σy - a*Σx) / n
    
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    for (size_t i = 0; i < n; ++i) {
      const double x = static_cast<double>(xs[i]);
      const double y = static_cast<double>(ys[i]);
      sum_x += x;
      sum_y += y;
      sum_xy += x * y;
      sum_x2 += x * x;
    }

    const double n_d = static_cast<double>(n);
    const double denom = (n_d * sum_x2 - sum_x * sum_x);
    
    float a, b;
    if (std::abs(denom) < 1e-10) {
      // Degenerate case: all x values are the same or too close
      a = 0.0f;
      b = static_cast<float>(sum_y / n_d);
    } else {
      a = static_cast<float>((n_d * sum_xy - sum_x * sum_y) / denom);
      b = static_cast<float>((sum_y - a * sum_x) / n_d);
    }

    M.segs.push_back({a, b, 0u, static_cast<uint32_t>(n)});
    return M;
  }

  /**
   * Find segment index for given x coordinate.
   * Simplified: always returns 0 (single segment).
   */
  inline uint32_t find_seg(uint32_t /*x*/) const {
    return 0u;
  }

  /**
   * Predict y value for given x coordinate.
   * 
   * @param x Input position
   * @return Predicted y value (may be negative or exceed true value)
   */
  inline int32_t predict(uint32_t x) const {
    if (segs.empty()) return 0;
    const PgmSeg& seg = segs[0];
    const float y = seg.a * static_cast<float>(x) + seg.b;
    return static_cast<int32_t>(std::round(y));
  }

  /**
   * Predict prefix count (alias for predict, for compatibility).
   */
  inline int32_t predict_prefix(uint32_t x) const {
    return predict(x);
  }
};

} // namespace cs

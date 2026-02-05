#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace cs {
struct SSA {
  uint32_t stride{32};
  std::vector<uint32_t> samples; // SA[i] for i % stride == 0
  uint32_t sample_at(uint32_t i) const {
    if (i % stride) throw std::runtime_error("not a sample index");
    return samples[i/stride];
  }
};
} // namespace cs

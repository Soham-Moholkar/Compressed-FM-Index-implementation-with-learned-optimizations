/**
 * veb.hpp — Cache-oblivious van Emde Boas layout for wavelet tree.
 * 
 * Purpose: Pack wavelet tree levels and their bit vectors into vEB-ordered
 *          macroblocks for improved cache locality during traversal.
 * 
 * Key Concepts:
 * - Macroblock: 4KB-aligned chunk containing bits + rank metadata for a subtree
 * - vEB ordering: Recursively partition tree to minimize cache misses
 * - Top-k levels: Store first k levels inline for fast access (k=2 or 3)
 * - Bottom levels: Pack in vEB order with 4KB alignment
 * 
 * Memory Layout:
 *   [Top levels (inline)] [Macroblock 0] [Macroblock 1] ... [Macroblock M-1]
 *   Each macroblock: [bits] [super_blocks] [sub_blocks] [padding to 4KB]
 */

#ifndef CS_LAYOUT_VEB_HPP
#define CS_LAYOUT_VEB_HPP

#include "../core/bitvector.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace cs {

// ──────────────────────────────────────────────────────────────
// Constants
// ──────────────────────────────────────────────────────────────

constexpr size_t VEB_MACROBLOCK_SIZE = 4096;  // 4KB per macroblock
constexpr size_t VEB_TOP_LEVELS = 2;           // Inline first 2 levels

// ──────────────────────────────────────────────────────────────
// Macroblock: 4KB-aligned unit containing a subtree's data
// ──────────────────────────────────────────────────────────────

struct Macroblock {
  std::vector<uint8_t> data;  // Actual payload (bits + metadata)
  size_t offset;              // Offset in final packed buffer
  
  Macroblock() : offset(0) {}
  
  // Pad to 4KB alignment.
  void pad_to_alignment() {
    const size_t remainder = data.size() % VEB_MACROBLOCK_SIZE;
    if (remainder != 0) {
      const size_t padding = VEB_MACROBLOCK_SIZE - remainder;
      data.resize(data.size() + padding, 0);
    }
  }
};

// ──────────────────────────────────────────────────────────────
// VebLayout: Transform linear wavelet tree into vEB order
// ──────────────────────────────────────────────────────────────

class VebLayout {
public:
  VebLayout() = default;

  // ─────────────────────────────────────────────────────────
  // build: Construct vEB layout from wavelet tree bit vectors
  // ─────────────────────────────────────────────────────────
  
  /**
   * Build vEB layout from wavelet tree levels.
   * 
   * @param levels: Array of BitVector, one per wavelet tree level (0=MSB, 7=LSB)
   * @param num_levels: Number of levels (typically 8 for byte alphabet)
   * @param top_k: Number of top levels to store inline (default 2)
   */
  void build(const BitVector* levels, size_t num_levels, size_t top_k = VEB_TOP_LEVELS);

  // ─────────────────────────────────────────────────────────
  // get_level_offset: Get byte offset for a specific level
  // ─────────────────────────────────────────────────────────
  
  /**
   * Get the byte offset for accessing level i in the packed buffer.
   * 
   * @param level: Level index (0=MSB, num_levels-1=LSB)
   * @return: Byte offset in packed_data_
   */
  size_t get_level_offset(size_t level) const;

  // ─────────────────────────────────────────────────────────
  // Access packed data
  // ─────────────────────────────────────────────────────────
  
  const uint8_t* data() const { return packed_data_.data(); }
  size_t size() const { return packed_data_.size(); }
  
  // Get pointer to a specific level's data.
  const uint8_t* level_data(size_t level) const {
    const size_t offset = get_level_offset(level);
    return (offset < packed_data_.size()) ? &packed_data_[offset] : nullptr;
  }

private:
  std::vector<uint8_t> packed_data_;    // Final vEB-ordered buffer
  std::vector<size_t> level_offsets_;   // Offset for each level
  size_t num_levels_;
  size_t top_k_;

  // Helper: Serialize a BitVector into a byte buffer.
  void serialize_bitvector(const BitVector& bv, std::vector<uint8_t>& out) const;
  
  // Helper: Compute vEB ordering for bottom levels.
  void compute_veb_order(size_t start_level, size_t num_bottom_levels,
                         std::vector<size_t>& order) const;
};

// ──────────────────────────────────────────────────────────────
// Implementation
// ──────────────────────────────────────────────────────────────

inline void VebLayout::build(const BitVector* levels, size_t num_levels, size_t top_k) {
  num_levels_ = num_levels;
  top_k_ = std::min(top_k, num_levels);
  
  level_offsets_.assign(num_levels, 0);
  packed_data_.clear();

  // 1) Serialize top-k levels inline (no vEB reordering).
  for (size_t i = 0; i < top_k_; ++i) {
    level_offsets_[i] = packed_data_.size();
    serialize_bitvector(levels[i], packed_data_);
  }

  // 2) Compute vEB ordering for bottom levels.
  const size_t num_bottom = num_levels - top_k_;
  if (num_bottom > 0) {
    std::vector<size_t> veb_order;
    compute_veb_order(top_k_, num_bottom, veb_order);

    // Serialize bottom levels in vEB order with 4KB alignment.
    for (size_t idx : veb_order) {
      const size_t level = top_k_ + idx;
      
      // Pad to 4KB boundary.
      const size_t remainder = packed_data_.size() % VEB_MACROBLOCK_SIZE;
      if (remainder != 0) {
        const size_t padding = VEB_MACROBLOCK_SIZE - remainder;
        packed_data_.resize(packed_data_.size() + padding, 0);
      }

      level_offsets_[level] = packed_data_.size();
      serialize_bitvector(levels[level], packed_data_);
    }
  }

  // 3) Final padding to 4KB alignment.
  const size_t remainder = packed_data_.size() % VEB_MACROBLOCK_SIZE;
  if (remainder != 0) {
    const size_t padding = VEB_MACROBLOCK_SIZE - remainder;
    packed_data_.resize(packed_data_.size() + padding, 0);
  }
}

inline size_t VebLayout::get_level_offset(size_t level) const {
  return (level < level_offsets_.size()) ? level_offsets_[level] : 0;
}

inline void VebLayout::serialize_bitvector(const BitVector& bv, std::vector<uint8_t>& out) const {
  // Serialize: [nbits (8 bytes)] [bits (words)] [super_blocks] [sub_blocks]
  
  const size_t nbits = bv.size();
  const auto& bits = bv.bits();
  const auto& super_blocks = bv.super_blocks();
  const auto& sub_blocks = bv.sub_blocks();

  // Write nbits (size_t = 8 bytes on x64).
  const uint8_t* nbits_ptr = reinterpret_cast<const uint8_t*>(&nbits);
  out.insert(out.end(), nbits_ptr, nbits_ptr + sizeof(size_t));

  // Write bits array (uint64_t words).
  const uint8_t* bits_ptr = reinterpret_cast<const uint8_t*>(bits.data());
  const size_t bits_bytes = bits.size() * sizeof(uint64_t);
  out.insert(out.end(), bits_ptr, bits_ptr + bits_bytes);

  // Write super_blocks (uint32_t).
  const uint8_t* super_ptr = reinterpret_cast<const uint8_t*>(super_blocks.data());
  const size_t super_bytes = super_blocks.size() * sizeof(uint32_t);
  out.insert(out.end(), super_ptr, super_ptr + super_bytes);

  // Write sub_blocks (uint16_t).
  const uint8_t* sub_ptr = reinterpret_cast<const uint8_t*>(sub_blocks.data());
  const size_t sub_bytes = sub_blocks.size() * sizeof(uint16_t);
  out.insert(out.end(), sub_ptr, sub_ptr + sub_bytes);
}

inline void VebLayout::compute_veb_order(size_t start_level, size_t num_bottom_levels,
                                         std::vector<size_t>& order) const {
  order.clear();
  if (num_bottom_levels == 0) return;

  // Simple vEB ordering: top-down recursive split.
  // For wavelet tree: first visit top half, then recursively bottom half.
  
  if (num_bottom_levels == 1) {
    order.push_back(0);
    return;
  }

  const size_t mid = num_bottom_levels / 2;
  
  // Visit top half.
  for (size_t i = 0; i < mid; ++i) {
    order.push_back(i);
  }
  
  // Visit bottom half.
  for (size_t i = mid; i < num_bottom_levels; ++i) {
    order.push_back(i);
  }
}

} // namespace cs

#endif // CS_LAYOUT_VEB_HPP

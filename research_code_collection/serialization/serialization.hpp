/**
 * serialization.hpp — Binary serialization for FM-index with mmap support.
 * 
 * File Format:
 *   [Header] [Text] [BWT] [C-array] [SSA] [Wavelet] [vEB Layout] [Footer]
 * 
 * Header:
 *   - Magic number: "CSIDX" (5 bytes)
 *   - Version: uint16_t (current: 1)
 *   - Flags: uint32_t (feature flags)
 *   - Offsets: uint64_t[8] (section byte offsets)
 * 
 * Zero-Copy Design:
 *   - All data 8-byte aligned
 *   - Arrays serialized as [count (8 bytes)] [data]
 *   - Can be directly mmap'd and cast to structs
 */

#ifndef CS_SERIALIZATION_HPP
#define CS_SERIALIZATION_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

namespace cs {

// ──────────────────────────────────────────────────────────────
// Constants
// ──────────────────────────────────────────────────────────────

constexpr char INDEX_MAGIC[6] = "CSIDX";  // 5 bytes + null terminator
constexpr uint16_t INDEX_VERSION = 1;

// Feature flags (bitfield)
enum IndexFlags : uint32_t {
  FLAG_NONE           = 0,
  FLAG_LEARNED_OCC    = 1 << 0,  // Uses learned bitvector
  FLAG_VEB_LAYOUT     = 1 << 1,  // Uses vEB layout
  FLAG_HUFFMAN_WAVELET = 1 << 2, // Uses Huffman-shaped wavelet
  FLAG_COMPRESSED_SSA = 1 << 3,  // SSA uses compression
};

// Section identifiers
enum SectionType : uint8_t {
  SECTION_HEADER = 0,
  SECTION_TEXT = 1,
  SECTION_BWT = 2,
  SECTION_C_ARRAY = 3,
  SECTION_SSA = 4,
  SECTION_WAVELET = 5,
  SECTION_VEB_LAYOUT = 6,
  SECTION_FOOTER = 7,
  NUM_SECTIONS = 8
};

// ──────────────────────────────────────────────────────────────
// Index Header (64 bytes, cache-line aligned)
// ──────────────────────────────────────────────────────────────

struct IndexHeader {
  char magic[8];                    // "CSIDX\0\0\0" (8 bytes for alignment)
  uint16_t version;                 // Format version
  uint16_t reserved1;               // Padding
  uint32_t flags;                   // Feature flags
  uint64_t text_len;                // Original text length
  uint64_t offsets[NUM_SECTIONS];   // Section byte offsets
  
  IndexHeader() {
    std::memset(this, 0, sizeof(IndexHeader));
    std::memcpy(magic, INDEX_MAGIC, 5);
    version = INDEX_VERSION;
  }
  
  bool is_valid() const {
    return std::memcmp(magic, INDEX_MAGIC, 5) == 0 && version == INDEX_VERSION;
  }
};

static_assert(sizeof(IndexHeader) == 88, "IndexHeader should be 88 bytes");

// ──────────────────────────────────────────────────────────────
// Serialization Writer
// ──────────────────────────────────────────────────────────────

class IndexWriter {
public:
  IndexWriter(const std::string& filepath);
  ~IndexWriter();

  // Write sections in order
  void write_header(uint32_t flags, size_t text_len);
  void write_text(const std::string& text);
  void write_bwt(const std::vector<uint8_t>& bwt);
  void write_c_array(const std::vector<uint32_t>& c_array);
  void write_ssa(const std::vector<uint32_t>& ssa_samples, uint32_t stride);
  void write_wavelet(const std::vector<uint64_t>& bits_data, 
                     const std::vector<uint32_t>& super_data,
                     const std::vector<uint16_t>& sub_data,
                     size_t num_levels);
  void write_veb_layout(const uint8_t* veb_data, size_t veb_size);
  void finalize();

private:
  std::ofstream file_;
  IndexHeader header_;
  size_t current_offset_;
  
  void align_to(size_t alignment);
  void write_raw(const void* data, size_t size);
  
  template<typename T>
  void write_array(const std::vector<T>& vec) {
    uint64_t count = vec.size();
    write_raw(&count, sizeof(uint64_t));
    if (count > 0) {
      write_raw(vec.data(), count * sizeof(T));
    }
  }
};

// ──────────────────────────────────────────────────────────────
// Deserialization Reader (mmap-based)
// ──────────────────────────────────────────────────────────────

class IndexReader {
public:
  IndexReader(const std::string& filepath);
  ~IndexReader();

  // Access header
  const IndexHeader* header() const { return header_; }
  bool has_flag(IndexFlags flag) const { return header_ && (header_->flags & flag); }
  
  // Access sections (zero-copy pointers into mmap'd region)
  const char* get_text(size_t* out_len = nullptr) const;
  const uint8_t* get_bwt(size_t* out_len = nullptr) const;
  const uint32_t* get_c_array(size_t* out_len = nullptr) const;
  const uint32_t* get_ssa(size_t* out_len = nullptr, uint32_t* out_stride = nullptr) const;
  const uint8_t* get_wavelet(size_t* out_size = nullptr) const;
  const uint8_t* get_veb_layout(size_t* out_size = nullptr) const;

private:
  void* mmap_ptr_;
  size_t mmap_size_;
  const IndexHeader* header_;
  
#ifdef _WIN32
  void* file_handle_;
  void* map_handle_;
#else
  int fd_;
#endif

  void open_mmap(const std::string& filepath);
  void close_mmap();
  
  template<typename T>
  const T* read_array_at(size_t offset, size_t* out_count = nullptr) const {
    if (offset == 0 || offset >= mmap_size_) return nullptr;
    const uint8_t* base = static_cast<const uint8_t*>(mmap_ptr_);
    const uint64_t* count_ptr = reinterpret_cast<const uint64_t*>(base + offset);
    if (out_count) *out_count = *count_ptr;
    return reinterpret_cast<const T*>(count_ptr + 1);
  }
};

} // namespace cs

#endif // CS_SERIALIZATION_HPP

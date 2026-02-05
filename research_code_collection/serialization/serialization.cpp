/**
 * serialization.cpp — Implementation of binary serialization for FM-index.
 */

#include "serialization.hpp"
#include <cstring>
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace cs {

// ──────────────────────────────────────────────────────────────
// IndexWriter Implementation
// ──────────────────────────────────────────────────────────────

IndexWriter::IndexWriter(const std::string& filepath)
  : file_(filepath, std::ios::binary | std::ios::out | std::ios::trunc),
    current_offset_(0) {
  if (!file_.is_open()) {
    throw std::runtime_error("Failed to open file for writing: " + filepath);
  }
  
  // Reserve space for header (will write at finalize)
  current_offset_ = sizeof(IndexHeader);
  file_.seekp(current_offset_);
}

IndexWriter::~IndexWriter() {
  if (file_.is_open()) {
    file_.close();
  }
}

void IndexWriter::align_to(size_t alignment) {
  const size_t remainder = current_offset_ % alignment;
  if (remainder != 0) {
    const size_t padding = alignment - remainder;
    const char zeros[64] = {0};
    for (size_t written = 0; written < padding; ) {
      const size_t chunk = (padding - written) < sizeof(zeros) ? (padding - written) : sizeof(zeros);
      write_raw(zeros, chunk);
    }
  }
}

void IndexWriter::write_raw(const void* data, size_t size) {
  file_.write(static_cast<const char*>(data), size);
  if (!file_) {
    throw std::runtime_error("Write failed");
  }
  current_offset_ += size;
}

void IndexWriter::write_header(uint32_t flags, size_t text_len) {
  header_.flags = flags;
  header_.text_len = text_len;
  header_.offsets[SECTION_HEADER] = 0;
}

void IndexWriter::write_text(const std::string& text) {
  align_to(8);
  header_.offsets[SECTION_TEXT] = current_offset_;
  
  uint64_t len = text.size();
  write_raw(&len, sizeof(uint64_t));
  write_raw(text.data(), len);
}

void IndexWriter::write_bwt(const std::vector<uint8_t>& bwt) {
  align_to(8);
  header_.offsets[SECTION_BWT] = current_offset_;
  write_array(bwt);
}

void IndexWriter::write_c_array(const std::vector<uint32_t>& c_array) {
  align_to(8);
  header_.offsets[SECTION_C_ARRAY] = current_offset_;
  write_array(c_array);
}

void IndexWriter::write_ssa(const std::vector<uint32_t>& ssa_samples, uint32_t stride) {
  align_to(8);
  header_.offsets[SECTION_SSA] = current_offset_;
  
  // Write stride first
  write_raw(&stride, sizeof(uint32_t));
  align_to(8);
  
  // Write samples
  write_array(ssa_samples);
}

void IndexWriter::write_wavelet(const std::vector<uint64_t>& bits_data,
                                const std::vector<uint32_t>& super_data,
                                const std::vector<uint16_t>& sub_data,
                                size_t num_levels) {
  align_to(8);
  header_.offsets[SECTION_WAVELET] = current_offset_;
  
  // Write num_levels
  uint64_t levels = num_levels;
  write_raw(&levels, sizeof(uint64_t));
  
  // Write arrays
  write_array(bits_data);
  write_array(super_data);
  write_array(sub_data);
}

void IndexWriter::write_veb_layout(const uint8_t* veb_data, size_t veb_size) {
  if (veb_data == nullptr || veb_size == 0) {
    header_.offsets[SECTION_VEB_LAYOUT] = 0;
    return;
  }
  
  align_to(4096);  // Align to page boundary
  header_.offsets[SECTION_VEB_LAYOUT] = current_offset_;
  
  uint64_t size = veb_size;
  write_raw(&size, sizeof(uint64_t));
  write_raw(veb_data, veb_size);
}

void IndexWriter::finalize() {
  align_to(8);
  header_.offsets[SECTION_FOOTER] = current_offset_;
  
  // Write footer marker
  const uint64_t footer_magic = 0x444E4553435300ULL;  // "CSEND\0\0\0" as uint64_t
  write_raw(&footer_magic, sizeof(uint64_t));
  
  // Go back and write header
  file_.seekp(0);
  file_.write(reinterpret_cast<const char*>(&header_), sizeof(IndexHeader));
  
  file_.close();
}

// ──────────────────────────────────────────────────────────────
// IndexReader Implementation
// ──────────────────────────────────────────────────────────────

IndexReader::IndexReader(const std::string& filepath)
  : mmap_ptr_(nullptr), mmap_size_(0), header_(nullptr) {
#ifdef _WIN32
  file_handle_ = INVALID_HANDLE_VALUE;
  map_handle_ = NULL;
#else
  fd_ = -1;
#endif
  
  open_mmap(filepath);
  
  // Validate header
  if (mmap_size_ < sizeof(IndexHeader)) {
    close_mmap();
    throw std::runtime_error("File too small to contain header");
  }
  
  header_ = static_cast<const IndexHeader*>(mmap_ptr_);
  if (!header_->is_valid()) {
    close_mmap();
    throw std::runtime_error("Invalid index file: bad magic or version");
  }
}

IndexReader::~IndexReader() {
  close_mmap();
}

void IndexReader::open_mmap(const std::string& filepath) {
#ifdef _WIN32
  // Windows CreateFileMapping API
  file_handle_ = CreateFileA(
    filepath.c_str(),
    GENERIC_READ,
    FILE_SHARE_READ,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );
  
  if (file_handle_ == INVALID_HANDLE_VALUE) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }
  
  LARGE_INTEGER file_size;
  if (!GetFileSizeEx(file_handle_, &file_size)) {
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to get file size");
  }
  mmap_size_ = static_cast<size_t>(file_size.QuadPart);
  
  map_handle_ = CreateFileMappingA(
    file_handle_,
    NULL,
    PAGE_READONLY,
    0, 0,
    NULL
  );
  
  if (map_handle_ == NULL) {
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to create file mapping");
  }
  
  mmap_ptr_ = MapViewOfFile(
    map_handle_,
    FILE_MAP_READ,
    0, 0,
    0
  );
  
  if (mmap_ptr_ == nullptr) {
    CloseHandle(map_handle_);
    CloseHandle(file_handle_);
    throw std::runtime_error("Failed to map view of file");
  }
  
#else
  // POSIX mmap
  fd_ = open(filepath.c_str(), O_RDONLY);
  if (fd_ < 0) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }
  
  struct stat sb;
  if (fstat(fd_, &sb) < 0) {
    close(fd_);
    throw std::runtime_error("Failed to stat file");
  }
  mmap_size_ = sb.st_size;
  
  mmap_ptr_ = mmap(nullptr, mmap_size_, PROT_READ, MAP_PRIVATE, fd_, 0);
  if (mmap_ptr_ == MAP_FAILED) {
    close(fd_);
    throw std::runtime_error("Failed to mmap file");
  }
#endif
}

void IndexReader::close_mmap() {
#ifdef _WIN32
  if (mmap_ptr_ != nullptr) {
    UnmapViewOfFile(mmap_ptr_);
    mmap_ptr_ = nullptr;
  }
  if (map_handle_ != NULL) {
    CloseHandle(map_handle_);
    map_handle_ = NULL;
  }
  if (file_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle_);
    file_handle_ = INVALID_HANDLE_VALUE;
  }
#else
  if (mmap_ptr_ != nullptr && mmap_ptr_ != MAP_FAILED) {
    munmap(mmap_ptr_, mmap_size_);
    mmap_ptr_ = nullptr;
  }
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
#endif
}

const char* IndexReader::get_text(size_t* out_len) const {
  const size_t offset = header_->offsets[SECTION_TEXT];
  if (offset == 0 || offset >= mmap_size_) {
    if (out_len) *out_len = 0;
    return nullptr;
  }
  
  const uint8_t* base = static_cast<const uint8_t*>(mmap_ptr_);
  const uint64_t* len_ptr = reinterpret_cast<const uint64_t*>(base + offset);
  if (out_len) *out_len = *len_ptr;
  return reinterpret_cast<const char*>(len_ptr + 1);
}

const uint8_t* IndexReader::get_bwt(size_t* out_len) const {
  return read_array_at<uint8_t>(header_->offsets[SECTION_BWT], out_len);
}

const uint32_t* IndexReader::get_c_array(size_t* out_len) const {
  return read_array_at<uint32_t>(header_->offsets[SECTION_C_ARRAY], out_len);
}

const uint32_t* IndexReader::get_ssa(size_t* out_len, uint32_t* out_stride) const {
  const size_t offset = header_->offsets[SECTION_SSA];
  if (offset == 0 || offset >= mmap_size_) {
    if (out_len) *out_len = 0;
    if (out_stride) *out_stride = 0;
    return nullptr;
  }
  
  const uint8_t* base = static_cast<const uint8_t*>(mmap_ptr_);
  const uint32_t* stride_ptr = reinterpret_cast<const uint32_t*>(base + offset);
  if (out_stride) *out_stride = *stride_ptr;
  
  // Stride is 4 bytes, next 8-byte boundary has the array
  const size_t array_offset = offset + 8;  // Aligned to 8
  return read_array_at<uint32_t>(array_offset, out_len);
}

const uint8_t* IndexReader::get_wavelet(size_t* out_size) const {
  const size_t offset = header_->offsets[SECTION_WAVELET];
  if (offset == 0 || offset >= mmap_size_) {
    if (out_size) *out_size = 0;
    return nullptr;
  }
  
  const uint8_t* base = static_cast<const uint8_t*>(mmap_ptr_);
  if (out_size) {
    // Calculate size from section start to next section or end
    const size_t next_offset = header_->offsets[SECTION_VEB_LAYOUT];
    *out_size = (next_offset > offset) ? (next_offset - offset) : 0;
  }
  return base + offset;
}

const uint8_t* IndexReader::get_veb_layout(size_t* out_size) const {
  return read_array_at<uint8_t>(header_->offsets[SECTION_VEB_LAYOUT], out_size);
}

} // namespace cs

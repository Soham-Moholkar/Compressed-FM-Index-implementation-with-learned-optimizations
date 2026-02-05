#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace cs {
inline std::string slurp(const std::string& path){
  std::ifstream f(path, std::ios::binary);
  if(!f) throw std::runtime_error("cannot open: " + path);
  return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}
inline void dump(const std::string& path, const void* data, size_t nbytes){
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if(!f) throw std::runtime_error("cannot write: " + path);
  f.write(reinterpret_cast<const char*>(data), nbytes);
}
inline void dump_str(const std::string& path, const std::string& s){
  dump(path, s.data(), s.size());
}
} // namespace cs

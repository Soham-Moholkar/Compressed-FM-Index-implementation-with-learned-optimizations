#pragma once
#include <cstdint>
#include <cstddef>
#include <bit>
#if defined(CS_AVX2) || defined(__SSE4_2__) || defined(__POPCNT__)
  #include <immintrin.h>
#endif
#if defined(_MSC_VER)
  #include <intrin.h>
#endif

namespace cs {
inline uint32_t popcount64(uint64_t x) {
#if defined(CS_AVX2) || defined(__SSE4_2__) || defined(__POPCNT__)
  // Use hardware popcnt when explicitly enabled/available
  return static_cast<uint32_t>(_mm_popcnt_u64(x));
#elif defined(_MSC_VER)
  // MSVC intrinsic (available on x64; on x86 requires appropriate arch flags)
  return static_cast<uint32_t>(__popcnt64(static_cast<unsigned __int64>(x)));
#elif defined(__GNUC__) || defined(__clang__)
  // GCC/Clang builtin
  return static_cast<uint32_t>(__builtin_popcountll(x));
#else
  // C++20 fallback
  return static_cast<uint32_t>(std::popcount(x));
#endif
}
} // namespace cs

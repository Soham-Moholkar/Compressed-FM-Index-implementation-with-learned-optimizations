#pragma once
/**
 * config.hpp — Feature flags and compile-time parameters for the FM-index.
 *
 * Each flag can be overridden via CMake with -DCS_xxx=1/0.
 */

// ──────────────────────────────────────────────────────────────
// FEATURE FLAGS
// ──────────────────────────────────────────────────────────────

/// Use learned PGM-based occ() instead of classic wavelet rank.
#ifndef CS_USE_LEARNED_OCC
  #define CS_USE_LEARNED_OCC 0
#endif

/// Use van Emde Boas (cache-oblivious) layout for bitvector macroblocks.
#ifndef CS_USE_VEB_LAYOUT
  #define CS_USE_VEB_LAYOUT 1
#endif

/// Use Huffman-shaped wavelet tree instead of balanced binary wavelet.
#ifndef CS_ENABLE_HUFFMAN_WAVELET
  #define CS_ENABLE_HUFFMAN_WAVELET 0
#endif

// ──────────────────────────────────────────────────────────────
// SAMPLING & STRIDE PARAMETERS
// ──────────────────────────────────────────────────────────────

/// Suffix Array sample stride (every SA_SAMPLE_STRIDE positions).
#ifndef CS_SA_SAMPLE_STRIDE
  #define CS_SA_SAMPLE_STRIDE 32
#endif

/// Coarse stride S for PGM/learned occ (in bits).
#ifndef CS_COARSE_STRIDE_S
  #define CS_COARSE_STRIDE_S 512
#endif

/// Micro stride s for residuals (in bits).
#ifndef CS_MICRO_STRIDE_s
  #define CS_MICRO_STRIDE_s 32
#endif

/// Maximum popcount tail touches for bounded-touch learned occ.
#ifndef CS_MAX_TAIL_POPCOUNTS_R
  #define CS_MAX_TAIL_POPCOUNTS_R 2
#endif

// ──────────────────────────────────────────────────────────────
// BITVECTOR PARAMETERS
// ──────────────────────────────────────────────────────────────

/// Super-block size in bits (absolute rank stored every SUPER_BLOCK_SIZE bits).
#ifndef CS_SUPER_BLOCK_SIZE
  #define CS_SUPER_BLOCK_SIZE 2048
#endif

/// Sub-block size in bits (relative rank within super-block).
#ifndef CS_SUB_BLOCK_SIZE
  #define CS_SUB_BLOCK_SIZE 256
#endif

static_assert(CS_SUPER_BLOCK_SIZE % CS_SUB_BLOCK_SIZE == 0,
              "Super-block size must be a multiple of sub-block size");
static_assert(CS_SUB_BLOCK_SIZE % 64 == 0,
              "Sub-block size must be a multiple of 64 (word size)");

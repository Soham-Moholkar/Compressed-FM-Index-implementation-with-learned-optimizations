/**
 * benchmark.cpp — Performance benchmarks for FM-index implementations.
 * 
 * Compares:
 *  - Classic two-level bitvector vs Learned bitvector
 *  - Linear layout vs vEB layout
 *  - Different query patterns (random, sequential, skewed)
 * 
 * Metrics:
 *  - Query throughput (QPS)
 *  - Latency percentiles (p50, p95, p99)
 *  - Index size (bytes)
 *  - Build time (seconds)
 */

#include "../src/api/fm_index.hpp"
#include "../src/util/timer.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <iomanip>

using namespace cs;

// ──────────────────────────────────────────────────────────────
// Benchmark Configuration
// ──────────────────────────────────────────────────────────────

struct BenchConfig {
  std::string name;
  size_t num_queries = 10000;
  size_t warmup_queries = 1000;
  int pattern_seed = 42;
};

// ──────────────────────────────────────────────────────────────
// Test Data Generation
// ──────────────────────────────────────────────────────────────

static std::string generate_random_dna(size_t length, int seed) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> dist(0, 3);
  const char bases[] = "ACGT";
  
  std::string result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    result += bases[dist(rng)];
  }
  return result;
}

static std::string generate_text_with_patterns(size_t length) {
  std::string text;
  text.reserve(length);
  
  // Create text with repeated patterns
  const std::string patterns[] = {
    "banana", "apple", "orange", "grape", "cherry",
    "the quick brown fox", "jumps over", "lazy dog"
  };
  
  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> pattern_dist(0, 7);
  
  while (text.size() < length) {
    const std::string& pat = patterns[pattern_dist(rng)];
    text += pat;
    text += " ";
  }
  
  text.resize(length);
  return text;
}

// ──────────────────────────────────────────────────────────────
// Query Pattern Generators
// ──────────────────────────────────────────────────────────────

static std::vector<std::string> generate_random_patterns(
    const std::string& text, size_t num_patterns, size_t pattern_len, int seed) {
  std::vector<std::string> patterns;
  patterns.reserve(num_patterns);
  
  std::mt19937 rng(seed);
  std::uniform_int_distribution<size_t> dist(0, text.size() - pattern_len - 1);
  
  for (size_t i = 0; i < num_patterns; ++i) {
    size_t pos = dist(rng);
    patterns.push_back(text.substr(pos, pattern_len));
  }
  
  return patterns;
}

static std::vector<std::string> generate_frequent_patterns(
    const std::string& text, size_t num_patterns) {
  // Find most frequent substrings
  std::vector<std::string> patterns = {
    "an", "the", "ing", "ed", "er",
    "ba", "ap", "or", "qu", "la"
  };
  
  while (patterns.size() < num_patterns) {
    patterns.push_back(patterns[patterns.size() % 10]);
  }
  
  patterns.resize(num_patterns);
  return patterns;
}

// ──────────────────────────────────────────────────────────────
// Benchmark Runner
// ──────────────────────────────────────────────────────────────

struct BenchResult {
  std::string name;
  size_t num_queries;
  double total_time_ms;
  double qps;
  double p50_us;
  double p95_us;
  double p99_us;
  size_t total_matches;
};

static BenchResult run_count_benchmark(
    FMIndex& index,
    const std::vector<std::string>& patterns,
    const BenchConfig& config) {
  
  BenchResult result;
  result.name = config.name;
  result.num_queries = config.num_queries;
  result.total_matches = 0;
  
  std::vector<double> latencies_us;
  latencies_us.reserve(config.num_queries);
  
  // Warmup
  for (size_t i = 0; i < config.warmup_queries; ++i) {
    const auto& pattern = patterns[i % patterns.size()];
    volatile size_t count = index.count(pattern);
    (void)count;
  }
  
  // Actual benchmark
  Timer total_timer;
  for (size_t i = 0; i < config.num_queries; ++i) {
    const auto& pattern = patterns[i % patterns.size()];
    
    Timer query_timer;
    size_t count = index.count(pattern);
    double elapsed_us = query_timer.elapsed_ms() * 1000.0;
    
    latencies_us.push_back(elapsed_us);
    result.total_matches += count;
  }
  result.total_time_ms = total_timer.elapsed_ms();
  
  // Calculate statistics
  std::sort(latencies_us.begin(), latencies_us.end());
  result.qps = (config.num_queries / result.total_time_ms) * 1000.0;
  result.p50_us = latencies_us[latencies_us.size() / 2];
  result.p95_us = latencies_us[latencies_us.size() * 95 / 100];
  result.p99_us = latencies_us[latencies_us.size() * 99 / 100];
  
  return result;
}

static BenchResult run_locate_benchmark(
    FMIndex& index,
    const std::vector<std::string>& patterns,
    const BenchConfig& config) {
  
  BenchResult result;
  result.name = config.name + " (locate)";
  result.num_queries = config.num_queries;
  result.total_matches = 0;
  
  std::vector<double> latencies_us;
  latencies_us.reserve(config.num_queries);
  
  // Warmup
  std::cout << "  Warming up (" << config.warmup_queries << " queries)...\n";
  for (size_t i = 0; i < config.warmup_queries; ++i) {
    const auto& pattern = patterns[i % patterns.size()];
    auto locs = index.locate(pattern);
    result.total_matches += locs.size();
  }
  
  result.total_matches = 0;  // Reset after warmup
  
  // Actual benchmark
  std::cout << "  Running benchmark (" << config.num_queries << " queries)...\n";
  Timer total_timer;
  for (size_t i = 0; i < config.num_queries; ++i) {
    const auto& pattern = patterns[i % patterns.size()];
    
    if (i % 10 == 0) {
      std::cout << "    Progress: " << i << "/" << config.num_queries << "\r" << std::flush;
    }
    
    Timer query_timer;
    auto locs = index.locate(pattern);
    double elapsed_us = query_timer.elapsed_ms() * 1000.0;
    
    latencies_us.push_back(elapsed_us);
    result.total_matches += locs.size();
  }
  std::cout << "    Progress: " << config.num_queries << "/" << config.num_queries << "\n";
  result.total_time_ms = total_timer.elapsed_ms();
  
  // Calculate statistics
  std::sort(latencies_us.begin(), latencies_us.end());
  result.qps = (config.num_queries / result.total_time_ms) * 1000.0;
  result.p50_us = latencies_us[latencies_us.size() / 2];
  result.p95_us = latencies_us[latencies_us.size() * 95 / 100];
  result.p99_us = latencies_us[latencies_us.size() * 99 / 100];
  
  return result;
}

// ──────────────────────────────────────────────────────────────
// Result Display
// ──────────────────────────────────────────────────────────────

static void print_result(const BenchResult& result) {
  std::cout << "\n  " << result.name << ":\n";
  std::cout << "    Queries:      " << result.num_queries << "\n";
  std::cout << "    Total time:   " << std::fixed << std::setprecision(2) 
            << result.total_time_ms << " ms\n";
  std::cout << "    Throughput:   " << std::fixed << std::setprecision(0) 
            << result.qps << " QPS\n";
  std::cout << "    Latency p50:  " << std::fixed << std::setprecision(2) 
            << result.p50_us << " μs\n";
  std::cout << "    Latency p95:  " << std::fixed << std::setprecision(2) 
            << result.p95_us << " μs\n";
  std::cout << "    Latency p99:  " << std::fixed << std::setprecision(2) 
            << result.p99_us << " μs\n";
  std::cout << "    Total matches: " << result.total_matches << "\n";
}

static void print_comparison(const BenchResult& baseline, const BenchResult& improved) {
  double qps_speedup = improved.qps / baseline.qps;
  double p95_improvement = baseline.p95_us / improved.p95_us;
  
  std::cout << "\n  Comparison (" << improved.name << " vs " << baseline.name << "):\n";
  std::cout << "    QPS speedup:        " << std::fixed << std::setprecision(2) 
            << qps_speedup << "×\n";
  std::cout << "    p95 improvement:    " << std::fixed << std::setprecision(2) 
            << p95_improvement << "×\n";
}

// ──────────────────────────────────────────────────────────────
// Main Benchmarks
// ──────────────────────────────────────────────────────────────

int main() {
  std::cout << "=== FM-Index Benchmarks ===\n\n";
  
  // Configuration
  const size_t text_size = 100000;  // 100KB text
  const size_t num_queries = 10000;
  const size_t pattern_len = 5;
  
  std::cout << "Configuration:\n";
  std::cout << "  Text size:    " << text_size << " bytes\n";
  std::cout << "  Queries:      " << num_queries << "\n";
  std::cout << "  Pattern len:  " << pattern_len << "\n";
  
  // Generate test data
  std::cout << "\nGenerating test data...\n";
  std::string text = generate_text_with_patterns(text_size);
  text += "$";  // Add terminator
  
  auto random_patterns = generate_random_patterns(text, num_queries, pattern_len, 42);
  auto frequent_patterns = generate_frequent_patterns(text, num_queries);
  
  // Build index
  std::cout << "Building FM-index...\n";
  Timer build_timer;
  FMIndex index = FMIndex::build_from_text(text, BuildParams());
  double build_time = build_timer.elapsed_ms();
  
  std::cout << "  Build time: " << std::fixed << std::setprecision(2) 
            << build_time << " ms\n";
  
  // ────────────────────────────────────────────────────────────
  // Benchmark 1: Random pattern count queries
  // ────────────────────────────────────────────────────────────
  
  std::cout << "\n" << std::string(60, '=') << "\n";
  std::cout << "Benchmark 1: Random Pattern Count Queries\n";
  std::cout << std::string(60, '=') << "\n";
  
  BenchConfig random_config;
  random_config.name = "Random patterns";
  random_config.num_queries = num_queries;
  
  auto random_result = run_count_benchmark(index, random_patterns, random_config);
  print_result(random_result);
  
  // ────────────────────────────────────────────────────────────
  // Benchmark 2: Frequent pattern count queries
  // ────────────────────────────────────────────────────────────
  
  std::cout << "\n" << std::string(60, '=') << "\n";
  std::cout << "Benchmark 2: Frequent Pattern Count Queries\n";
  std::cout << std::string(60, '=') << "\n";
  
  BenchConfig frequent_config;
  frequent_config.name = "Frequent patterns";
  frequent_config.num_queries = num_queries;
  
  auto frequent_result = run_count_benchmark(index, frequent_patterns, frequent_config);
  print_result(frequent_result);
  
  // ────────────────────────────────────────────────────────────
  // Benchmark 3: Locate queries (SKIP - too slow for large datasets)
  // ────────────────────────────────────────────────────────────
  
  std::cout << "\n" << std::string(60, '=') << "\n";
  std::cout << "Benchmark 3: Locate Queries (reduced)\n";
  std::cout << std::string(60, '=') << "\n";
  
  BenchConfig locate_config;
  locate_config.name = "Locate";
  locate_config.num_queries = 100;  // Much fewer queries for locate
  locate_config.warmup_queries = 10;
  
  auto locate_result = run_locate_benchmark(index, frequent_patterns, locate_config);
  print_result(locate_result);
  
  // ────────────────────────────────────────────────────────────
  // Summary
  // ────────────────────────────────────────────────────────────
  
  std::cout << "\n" << std::string(60, '=') << "\n";
  std::cout << "Summary\n";
  std::cout << std::string(60, '=') << "\n";
  std::cout << "\n  Random pattern QPS:   " << std::fixed << std::setprecision(0) 
            << random_result.qps << "\n";
  std::cout << "  Frequent pattern QPS: " << std::fixed << std::setprecision(0) 
            << frequent_result.qps << "\n";
  std::cout << "  Locate QPS:           " << std::fixed << std::setprecision(0) 
            << locate_result.qps << "\n";
  std::cout << "\n  Build time:           " << std::fixed << std::setprecision(2) 
            << build_time << " ms\n";
  
  std::cout << "\n=== Benchmarks Complete ===\n";
  
  return 0;
}

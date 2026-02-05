#pragma once
#include <chrono>
#include <string>
#include <iostream>

namespace cs {

// Simple timer for benchmarking
class Timer {
public:
  Timer() : start_(std::chrono::steady_clock::now()) {}
  
  void reset() {
    start_ = std::chrono::steady_clock::now();
  }
  
  double elapsed_ms() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start_).count();
  }
  
  double elapsed_us() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(now - start_).count();
  }

private:
  std::chrono::steady_clock::time_point start_;
};

// Scope-based timer that prints on destruction
struct ScopeTimer {
  std::string name; 
  std::chrono::steady_clock::time_point t0;
  
  explicit ScopeTimer(std::string n): name(std::move(n)), t0(std::chrono::steady_clock::now()) {}
  
  ~ScopeTimer(){
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0).count();
    std::cerr << "[TIMER] " << name << ": " << ms << " ms\n";
  }
};

} // namespace cs

# Research Code Collection - README

## ✅ Collection Complete

This folder contains **all important source code** from the compressed search engine project, organized for research paper reference.

---

## 📂 What's Included

✓ **All core implementation files** (bitvectors, wavelet trees, BWT, FM-Index)  
✓ **Tool and utility source code** (build, query, benchmark)  
✓ **Test suites** with examples and validation  
✓ **Configuration files** (CMake, headers)  
✓ **Comprehensive documentation** (9 detailed markdown files)

**Nothing has been modified** - all files are exact copies from the original project.

---

## 📖 Documentation Index

### Start Here
- **`00_RESEARCH_INDEX.md`** - Overview and navigation guide

### Core Documentation (Read in Order)
1. **`01_CORE_STRUCTURES.md`** - Bitvectors, wavelet trees, BWT, suffix arrays
2. **`02_API_INTERFACE.md`** - FM-Index API and usage
3. **`03_LEARNED_STRUCTURES.md`** - Machine learning enhancements (PGM-Index)
4. **`04_OPTIMIZATION_TECHNIQUES.md`** - VEB layout, serialization, performance
5. **`05_TOOLS_AND_UTILITIES.md`** - Command-line tools and helpers
6. **`06_TESTING_FRAMEWORK.md`** - Test suite and validation (THIS FILE)
7. **`07_BUILD_AND_USAGE.md`** - Compilation and execution instructions
8. **`08_RESEARCH_CONCEPTS.md`** - Theoretical background and algorithms
9. **`09_FILE_BY_FILE_GUIDE.md`** - Quick reference for every file

---

## 🎯 Quick Navigation by Research Topic

### Compression & Indexing
- BWT theory → `08_RESEARCH_CONCEPTS.md` (Section 1.3)
- FM-Index algorithm → `02_API_INTERFACE.md`
- Space complexity → `08_RESEARCH_CONCEPTS.md` (Section 3.2)

### Data Structures
- Rank/select → `01_CORE_STRUCTURES.md` (Sections 1-2)
- Wavelet trees → `01_CORE_STRUCTURES.md` (Section 3)
- Suffix arrays → `01_CORE_STRUCTURES.md` (Section 6-7)

### Machine Learning
- Learned indexes → `03_LEARNED_STRUCTURES.md`
- PGM theory → `08_RESEARCH_CONCEPTS.md` (Section 5.2)

### Performance
- Cache optimization → `04_OPTIMIZATION_TECHNIQUES.md`
- Benchmarking → `05_TOOLS_AND_UTILITIES.md` (Section 3)
- Complexity analysis → `08_RESEARCH_CONCEPTS.md` (Section 4.2)

### Implementation
- Build instructions → `07_BUILD_AND_USAGE.md`
- Testing → `06_TESTING_FRAMEWORK.md`
- File reference → `09_FILE_BY_FILE_GUIDE.md`

---

## 📊 Code Organization

```
research_code_collection/
│
├── 📄 Documentation (9 files, ~25,000 words)
│   ├── 00_RESEARCH_INDEX.md .................. Start here
│   ├── 01_CORE_STRUCTURES.md ................ Data structures
│   ├── 02_API_INTERFACE.md .................. FM-Index API
│   ├── 03_LEARNED_STRUCTURES.md ............. Machine learning
│   ├── 04_OPTIMIZATION_TECHNIQUES.md ........ Performance
│   ├── 05_TOOLS_AND_UTILITIES.md ............ Tools & helpers
│   ├── 06_TESTING_FRAMEWORK.md .............. Tests (THIS FILE!)
│   ├── 07_BUILD_AND_USAGE.md ................ Build & run
│   ├── 08_RESEARCH_CONCEPTS.md .............. Theory
│   └── 09_FILE_BY_FILE_GUIDE.md ............. File reference
│
├── 📁 core/ (12 files, ~2,800 lines)
│   ├── bitvector.cpp/hpp .................... Rank/select structures
│   ├── bitvector_learned.cpp/hpp ............ Learned bitvectors
│   ├── wavelet.cpp/hpp ...................... Wavelet trees
│   ├── wavelet_learned.cpp/hpp .............. Learned wavelets
│   ├── bwt.hpp .............................. Burrows-Wheeler Transform
│   ├── sais.hpp ............................. Suffix array (SAIS)
│   └── ssa.cpp/hpp .......................... Sampled suffix array
│
├── 📁 api/ (2 files, ~600 lines)
│   └── fm_index.cpp/hpp ..................... Main FM-Index implementation
│
├── 📁 learned/ (1 file, ~350 lines)
│   └── pgm.hpp .............................. PGM-Index (learned structure)
│
├── 📁 layout/ (1 file, ~200 lines)
│   └── veb.hpp .............................. Van Emde Boas layout
│
├── 📁 serialization/ (2 files, ~300 lines)
│   └── serialization.cpp/hpp ................ Save/load indices
│
├── 📁 util/ (3 files, ~370 lines)
│   ├── io.hpp ............................... File I/O helpers
│   ├── timer.hpp ............................ High-precision timing
│   └── bitops.hpp ........................... Bit manipulation
│
├── 📁 tools/ (4 files, ~1,450 lines)
│   ├── build_index.cpp ...................... Build index CLI
│   ├── query_cli.cpp ........................ Query index CLI
│   ├── benchmark.cpp ........................ Automated benchmarks
│   └── bench.cpp ............................ Micro-benchmarks
│
├── 📁 tests/ (10 files, ~2,850 lines)
│   ├── bitvector_tests.cpp .................. Bitvector tests
│   ├── wavelet_tests.cpp .................... Wavelet tests
│   ├── fm_search_tests.cpp .................. FM-Index tests
│   ├── serialization_tests.cpp .............. Serialization tests
│   ├── veb_layout_tests.cpp ................. VEB layout tests
│   ├── learned_occ_tests.cpp ................ Learned structure tests
│   └── ... (other test files)
│
├── config.hpp ............................... Configuration constants
└── CMakeLists.txt ........................... Build configuration
```

**Total: 38 source files, ~9,190 lines of code**

---

## 🎓 For Your Research Paper

### What This Collection Provides

✅ **Implementation Reference**: Cite specific algorithms and data structures  
✅ **Performance Data**: Benchmark tools for generating results  
✅ **Theoretical Background**: Complete algorithmic analysis  
✅ **Code Examples**: Copy snippets for explanations  
✅ **Validation**: Test suites prove correctness  

### How to Use

1. **Understand Theory**: Start with `08_RESEARCH_CONCEPTS.md`
2. **Study Implementation**: Read `01_CORE_STRUCTURES.md` and `02_API_INTERFACE.md`
3. **Run Experiments**: Follow `07_BUILD_AND_USAGE.md` to compile and test
4. **Generate Data**: Use tools in `tools/` for benchmarking
5. **Write Paper**: Reference specific files and algorithms

### Key Topics to Cover

- **String Algorithms**: Suffix arrays, BWT, FM-Index
- **Succinct Data Structures**: Rank/select, wavelet trees
- **Learned Structures**: PGM-Index, learned bitvectors
- **Cache Optimization**: VEB layout, memory efficiency
- **Compression**: Space bounds, entropy coding
- **Performance Engineering**: Benchmarking, profiling

---

## 📚 Recommended Reading Order

### For Understanding (3-4 hours)
1. `00_RESEARCH_INDEX.md` (10 min)
2. `08_RESEARCH_CONCEPTS.md` (60 min)
3. `01_CORE_STRUCTURES.md` (45 min)
4. `02_API_INTERFACE.md` (45 min)
5. Skim other documentation as needed

### For Implementation (2-3 hours)
1. `09_FILE_BY_FILE_GUIDE.md` (15 min)
2. Read `core/bitvector.cpp` (30 min)
3. Read `core/wavelet.cpp` (30 min)
4. Read `api/fm_index.cpp` (45 min)
5. Run examples from `07_BUILD_AND_USAGE.md` (45 min)

### For Experiments (varies)
1. `07_BUILD_AND_USAGE.md` - Build project
2. `05_TOOLS_AND_UTILITIES.md` - Understand tools
3. Run benchmarks, collect data
4. Analyze results

---

## 🔬 Research Applications

### Suitable For

- **Undergraduate Projects**: Implement and analyze data structures
- **Master's Thesis**: Extend with new optimizations or applications
- **PhD Research**: Novel learned structures, distributed indexing
- **Course Projects**: Demonstrate compression and search algorithms
- **Technical Reports**: Document practical implementations

### Topics for Papers

1. Performance comparison of learned vs traditional structures
2. Cache-oblivious algorithms in practice
3. Space-time trade-offs in compressed indices
4. FM-Index applications to genomics/text mining
5. Succinct data structures implementation study

---

## 💻 System Requirements

**To compile and run the code:**

- **OS**: Windows, Linux, or macOS
- **Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
- **Build Tool**: CMake 3.12+
- **RAM**: 4GB minimum (8GB+ recommended for large datasets)
- **Disk**: 1GB for build artifacts

**To read documentation only:**

- Any text editor or markdown viewer
- No compilation needed!

---

## ⚠️ Important Notes

### What This Is

✓ **Research reference** - Code copied for study and paper writing  
✓ **Documentation** - Comprehensive explanations of all components  
✓ **Self-contained** - All important files included  

### What This Is NOT

✗ **Not a tutorial** - Assumes CS background (algorithms, data structures)  
✗ **Not production-ready** - Research code, not hardened for deployment  
✗ **Not modified** - Exact copies from original project  

---

## 📧 Questions?

If you need clarification on any component:

1. Check the relevant documentation file
2. Look up the concept in `08_RESEARCH_CONCEPTS.md`
3. Find the file in `09_FILE_BY_FILE_GUIDE.md`
4. Read the source code comments

---

## 🎯 Success Checklist

Use this to track your research progress:

- [ ] Read `00_RESEARCH_INDEX.md` for overview
- [ ] Understand core concepts from `08_RESEARCH_CONCEPTS.md`
- [ ] Study data structures in `01_CORE_STRUCTURES.md`
- [ ] Learn FM-Index from `02_API_INTERFACE.md`
- [ ] Build and run examples (`07_BUILD_AND_USAGE.md`)
- [ ] Run benchmarks and collect data
- [ ] Write paper sections referencing implementations
- [ ] Cite original papers (see `08_RESEARCH_CONCEPTS.md` Section 8)
- [ ] Include performance graphs and analysis
- [ ] Submit and publish! 🎉

---

## 📊 Quick Stats

| Metric | Value |
|--------|-------|
| **Total Files** | 38 source files + 9 docs |
| **Code Lines** | ~9,190 lines |
| **Documentation Words** | ~25,000 words |
| **Documentation Pages** | ~150 pages (printed) |
| **Topics Covered** | 50+ algorithms and concepts |
| **Test Cases** | 100+ unit tests |
| **Benchmarks** | Micro + macro performance tests |

---

## 🚀 Final Words

This collection represents a **complete compressed search engine implementation** with production-quality code and research-grade documentation.

**Everything you need for your research paper is here.**

- Want to understand **how it works**? → Read the docs
- Want to see **the implementation**? → Check the code
- Want to **run experiments**? → Use the tools
- Want to **validate correctness**? → Run the tests

**Good luck with your research! 🎓**

---

**Created**: October 28, 2025  
**Purpose**: Research paper reference and academic study  
**Status**: ✅ Complete and ready to use

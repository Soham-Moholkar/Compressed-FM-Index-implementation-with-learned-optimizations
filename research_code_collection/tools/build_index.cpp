#include "../src/api/fm_index.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

void print_usage() {
    std::cout << "Usage: build_index <input_text_file> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --no-terminator    Don't add $ terminator (use if file already has one)\n";
    std::cout << "  --stats            Show detailed statistics\n\n";
    std::cout << "Example:\n";
    std::cout << "  build_index mybook.txt\n";
    std::cout << "  build_index genome.txt --no-terminator --stats\n";
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string input_file = argv[1];
    bool add_terminator = true;
    bool show_stats = false;

    // Parse options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--no-terminator") {
            add_terminator = false;
        } else if (arg == "--stats") {
            show_stats = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    try {
        std::cout << "Reading text from: " << input_file << "\n";
        std::string text = read_file(input_file);
        
        if (text.empty()) {
            std::cerr << "Error: File is empty\n";
            return 1;
        }

        std::cout << "Text size: " << text.size() << " bytes\n";

        // Add terminator if needed
        if (add_terminator && text.back() != '$' && text.back() != '\0') {
            text += '$';
            std::cout << "Added terminator '$'\n";
        }

        // Build index
        std::cout << "\nBuilding FM-index...\n";
        auto start = std::chrono::high_resolution_clock::now();
        
        cs::BuildParams params;
        params.ssa_stride = 32;
        cs::FMIndex index = cs::FMIndex::build_from_text(text, params);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Index built successfully in " << duration.count() << " ms\n";

        if (show_stats) {
            std::cout << "\n=== Index Statistics ===\n";
            std::cout << "Text length: " << text.size() << " bytes\n";
        }

        // Interactive query loop
        std::cout << "\n=== Ready for Queries ===\n";
        std::cout << "Enter patterns to search (or 'quit' to exit):\n\n";

        std::string pattern;
        while (true) {
            std::cout << "Pattern> ";
            std::getline(std::cin, pattern);

            if (pattern == "quit" || pattern == "exit" || pattern == "q") {
                break;
            }

            if (pattern.empty()) {
                continue;
            }

            try {
                auto query_start = std::chrono::high_resolution_clock::now();
                size_t count = index.count(pattern);
                auto query_end = std::chrono::high_resolution_clock::now();
                auto query_time = std::chrono::duration_cast<std::chrono::microseconds>(query_end - query_start);

                std::cout << "  Count: " << count << " occurrences";
                std::cout << " (query time: " << query_time.count() << " μs)\n";

                if (count > 0 && count <= 10) {
                    std::cout << "  Finding positions...\n";
                    auto positions = index.locate(pattern);
                    std::cout << "  Positions: ";
                    for (size_t i = 0; i < positions.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << positions[i];
                    }
                    std::cout << "\n";
                } else if (count > 10) {
                    std::cout << "  (Too many matches to show positions - use locate for specific queries)\n";
                }

            } catch (const std::exception& e) {
                std::cerr << "  Error: " << e.what() << "\n";
            }

            std::cout << "\n";
        }

        std::cout << "Goodbye!\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

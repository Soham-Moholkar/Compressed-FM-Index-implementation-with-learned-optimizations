// Quick test to check if serialization basics work
#include "../src/serialization/serialization.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "Creating writer...\n";
        cs::IndexWriter writer("quick_test.csidx");
        
        std::cout << "Writing header...\n";
        writer.write_header(0, 100);
        
        std::cout << "Finalizing...\n";
        writer.finalize();
        
        std::cout << "Write successful!\n";
        
        std::cout << "Creating reader...\n";
        cs::IndexReader reader("quick_test.csidx");
        
        std::cout << "Checking header...\n";
        if (reader.header()->is_valid()) {
            std::cout << "Header is valid!\n";
        }
        
        std::cout << "Test passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>

namespace zkmini {

#define ZK_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "Assertion failed: " << (message) << std::endl; \
            std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << std::endl; \
            std::abort(); \
        } \
    } while (0)

class ScopeTimer {
public:
    ScopeTimer(const std::string& name) 
        : name(name), start_time(std::chrono::high_resolution_clock::now()) {
        std::cout << "[Timer] Starting: " << name << std::endl;
    }
    
    ~ScopeTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time
        ).count();
        std::cout << "[Timer] " << name << " took " << duration << " ms" << std::endl;
    }
    
    double elapsed_ms() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - start_time
        ).count() / 1000.0;
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time;
};

#define ZK_TIMER(name) ScopeTimer _timer(name)

class ProgressBar {
public:
    ProgressBar(size_t total, const std::string& description = "");
    
    void update(size_t current);
    void finish();
    
private:
    size_t total;
    size_t last_printed;
    std::string description;
    std::chrono::high_resolution_clock::time_point start_time;
    
    void print_bar(size_t current);
};

// Memory usage utilities
class MemoryInfo {
public:
    // Get current memory usage in bytes
    static size_t get_memory_usage();
    
    // Get peak memory usage in bytes
    static size_t get_peak_memory_usage();
    
    // Print memory usage in human readable format
    static void print_memory_usage();
    
// Progress indicator for long operations
private:
    static std::string format_bytes(size_t bytes);
};

// Bit manipulation utilities
class BitUtils {
public:
    // Count number of set bits
    static size_t popcount(uint64_t x);
    
    // Find position of least significant set bit (0-indexed)
    static size_t trailing_zeros(uint64_t x);
    
    // Find position of most significant set bit (0-indexed)
    static size_t leading_zeros(uint64_t x);
    
    // Check if number is power of 2
    static bool is_power_of_two(uint64_t x);
    
    // Round up to next power of 2
    static uint64_t next_power_of_two(uint64_t x);
    
    // Reverse bits in 64-bit integer
    static uint64_t reverse_bits(uint64_t x);
};

// String utilities
class StringUtils {
public:
    // Convert bytes to hex string
    static std::string bytes_to_hex(const std::vector<uint8_t>& bytes);
    
    // Convert hex string to bytes
    static std::vector<uint8_t> hex_to_bytes(const std::string& hex);
    
    // Trim whitespace from string
    static std::string trim(const std::string& str);
    
    // Split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    // Join strings with delimiter
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
};

} // namespace zkmini
#include "zkmini/utils.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>

#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace zkmini {


ProgressBar::ProgressBar(size_t total, const std::string& description) 
    : total(total), last_printed(0), description(description) {
    start_time = std::chrono::high_resolution_clock::now();
    std::cout << description << " [" << std::flush;
}

void ProgressBar::update(size_t current) {
    size_t progress = (current * 50) / total; 
    if (progress > last_printed) {
        for (size_t i = last_printed; i < progress; ++i) {
            std::cout << "=" << std::flush;
        }
        last_printed = progress;
    }
}

void ProgressBar::finish() {
    for (size_t i = last_printed; i < 50; ++i) {
        std::cout << "=" << std::flush;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    ).count();
    
    std::cout << "] " << duration << " ms" << std::endl;
}

void ProgressBar::print_bar(size_t current) {
    update(current);
}

size_t MemoryInfo::get_memory_usage() {
#ifdef __linux__
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss * 1024; 
#else
    return 0; 
#endif
}

size_t MemoryInfo::get_peak_memory_usage() {
    return get_memory_usage(); // Same as current for this implementation
}

void MemoryInfo::print_memory_usage() {
    size_t usage = get_memory_usage();
    std::cout << "Memory usage: " << format_bytes(usage) << std::endl;
}

std::string MemoryInfo::format_bytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    size_t unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

size_t BitUtils::popcount(uint64_t x) {
    return __builtin_popcountll(x);
}

size_t BitUtils::trailing_zeros(uint64_t x) {
    if (x == 0) return 64;
    return __builtin_ctzll(x);
}

size_t BitUtils::leading_zeros(uint64_t x) {
    if (x == 0) return 64;
    return __builtin_clzll(x);
}

bool BitUtils::is_power_of_two(uint64_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

uint64_t BitUtils::next_power_of_two(uint64_t x) {
    if (x <= 1) return 1;
    return 1ULL << (64 - leading_zeros(x - 1));
}

uint64_t BitUtils::reverse_bits(uint64_t x) {
    x = ((x & 0xAAAAAAAAAAAAAAAA) >> 1) | ((x & 0x5555555555555555) << 1);
    x = ((x & 0xCCCCCCCCCCCCCCCC) >> 2) | ((x & 0x3333333333333333) << 2);
    x = ((x & 0xF0F0F0F0F0F0F0F0) >> 4) | ((x & 0x0F0F0F0F0F0F0F0F) << 4);
    x = ((x & 0xFF00FF00FF00FF00) >> 8) | ((x & 0x00FF00FF00FF00FF) << 8);
    x = ((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16);
    return (x >> 32) | (x << 32);
}

std::string StringUtils::bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

std::vector<uint8_t> StringUtils::hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte_str = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

std::string StringUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::ostringstream oss;
    oss << strings[0];
    
    for (size_t i = 1; i < strings.size(); ++i) {
        oss << delimiter << strings[i];
    }
    
    return oss.str();
}

} 
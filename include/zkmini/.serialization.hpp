#pragma once

#include ".keys.hpp"
#include ".groth16.hpp"
#include <string>
#include <vector>
#include <fstream>

namespace zkmini {

// Placeholder for serialization utilities
// TODO: Implement binary and JSON serialization
class Serialization {
public:
    // Binary serialization helpers
    template<typename T>
    static std::vector<uint8_t> serialize_vector(const std::vector<T>& vec);
    
    template<typename T>
    static std::vector<T> deserialize_vector(const std::vector<uint8_t>& data, size_t& offset);
    
    // Field element serialization
    static std::vector<uint8_t> serialize_fr(const Fr& element);
    static Fr deserialize_fr(const std::vector<uint8_t>& data, size_t& offset);
    
    // Group element serialization  
    static std::vector<uint8_t> serialize_g1(const G1& point);
    static G1 deserialize_g1(const std::vector<uint8_t>& data, size_t& offset);
    
    static std::vector<uint8_t> serialize_g2(const G2& point);
    static G2 deserialize_g2(const std::vector<uint8_t>& data, size_t& offset);
    
    // File I/O utilities
    static void write_file(const std::string& filename, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> read_file(const std::string& filename);
    
    // JSON utilities
    static std::string fr_to_json(const Fr& element);
    static Fr fr_from_json(const std::string& json_str);
    
    static std::string g1_to_json(const G1& point);
    static G1 g1_from_json(const std::string& json_str);
    
    static std::string g2_to_json(const G2& point);
    static G2 g2_from_json(const std::string& json_str);
    
    // Compressed point serialization (for efficiency)
    static std::vector<uint8_t> serialize_g1_compressed(const G1& point);
    static G1 deserialize_g1_compressed(const std::vector<uint8_t>& data, size_t& offset);
    
    static std::vector<uint8_t> serialize_g2_compressed(const G2& point);
    static G2 deserialize_g2_compressed(const std::vector<uint8_t>& data, size_t& offset);

private:
    // Little-endian integer serialization
    static void write_uint32(std::vector<uint8_t>& data, uint32_t value);
    static void write_uint64(std::vector<uint8_t>& data, uint64_t value);
    static uint32_t read_uint32(const std::vector<uint8_t>& data, size_t& offset);
    static uint64_t read_uint64(const std::vector<uint8_t>& data, size_t& offset);
    
    // Size helpers
    static constexpr size_t FR_SIZE = 32;  // 256 bits
    static constexpr size_t G1_SIZE = 64;  // 2 * 256 bits (x, y)
    static constexpr size_t G2_SIZE = 128; // 4 * 256 bits (x0, x1, y0, y1)
    static constexpr size_t G1_COMPRESSED_SIZE = 33; // 256 + 1 bit
    static constexpr size_t G2_COMPRESSED_SIZE = 65; // 512 + 1 bit
};

} // namespace zkmini
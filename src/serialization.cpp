#include "zkmini/serialization.hpp"
#include "zkmini/utils.hpp"
#include <fstream>

namespace zkmini {

// Vector serialization
template<typename T>
std::vector<uint8_t> Serialization::serialize_vector(const std::vector<T>& vec) {
    std::vector<uint8_t> result;
    
    // Write vector size
    write_uint64(result, vec.size());
    
    // Write elements (assuming T has serialize method)
    for (const auto& element : vec) {
        auto element_data = element.serialize();
        result.insert(result.end(), element_data.begin(), element_data.end());
    }
    
    return result;
}

template<typename T>
std::vector<T> Serialization::deserialize_vector(const std::vector<uint8_t>& data, size_t& offset) {
    // Read vector size
    uint64_t size = read_uint64(data, offset);
    
    std::vector<T> result;
    result.reserve(size);
    
    // Read elements
    for (uint64_t i = 0; i < size; ++i) {
        result.push_back(T::deserialize(data, offset));
    }
    
    return result;
}

// Field element serialization
std::vector<uint8_t> Serialization::serialize_fr(const Fr& element) {
    // TODO: Implement Fr serialization (32 bytes)
    std::vector<uint8_t> result(FR_SIZE, 0);
    return result;
}

Fr Serialization::deserialize_fr(const std::vector<uint8_t>& data, size_t& offset) {
    // TODO: Implement Fr deserialization
    ZK_ASSERT(offset + FR_SIZE <= data.size(), "Not enough data for Fr");
    offset += FR_SIZE;
    return Fr();
}

// Group element serialization
std::vector<uint8_t> Serialization::serialize_g1(const G1& point) {
    // TODO: Implement G1 serialization (64 bytes for affine coordinates)
    std::vector<uint8_t> result(G1_SIZE, 0);
    return result;
}

G1 Serialization::deserialize_g1(const std::vector<uint8_t>& data, size_t& offset) {
    // TODO: Implement G1 deserialization
    ZK_ASSERT(offset + G1_SIZE <= data.size(), "Not enough data for G1");
    offset += G1_SIZE;
    return G1();
}

std::vector<uint8_t> Serialization::serialize_g2(const G2& point) {
    // TODO: Implement G2 serialization (128 bytes for affine coordinates)
    std::vector<uint8_t> result(G2_SIZE, 0);
    return result;
}

G2 Serialization::deserialize_g2(const std::vector<uint8_t>& data, size_t& offset) {
    // TODO: Implement G2 deserialization
    ZK_ASSERT(offset + G2_SIZE <= data.size(), "Not enough data for G2");
    offset += G2_SIZE;
    return G2();
}

// File I/O utilities
void Serialization::write_file(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    ZK_ASSERT(file.is_open(), "Failed to open file for writing: " + filename);
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
}

std::vector<uint8_t> Serialization::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    ZK_ASSERT(file.is_open(), "Failed to open file for reading: " + filename);
    
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(file_size);
    file.read(reinterpret_cast<char*>(data.data()), file_size);
    file.close();
    
    return data;
}

// JSON utilities
std::string Serialization::fr_to_json(const Fr& element) {
    // TODO: Implement Fr to JSON conversion
    return "\"0x0000000000000000000000000000000000000000000000000000000000000000\"";
}

Fr Serialization::fr_from_json(const std::string& json_str) {
    // TODO: Implement Fr from JSON conversion
    return Fr();
}

std::string Serialization::g1_to_json(const G1& point) {
    // TODO: Implement G1 to JSON conversion
    return "{\"x\":\"0x0\",\"y\":\"0x0\"}";
}

G1 Serialization::g1_from_json(const std::string& json_str) {
    // TODO: Implement G1 from JSON conversion
    return G1();
}

std::string Serialization::g2_to_json(const G2& point) {
    // TODO: Implement G2 to JSON conversion
    return "{\"x\":[\"0x0\",\"0x0\"],\"y\":[\"0x0\",\"0x0\"]}";
}

G2 Serialization::g2_from_json(const std::string& json_str) {
    // TODO: Implement G2 from JSON conversion
    return G2();
}

// Compressed serialization
std::vector<uint8_t> Serialization::serialize_g1_compressed(const G1& point) {
    // TODO: Implement compressed G1 serialization
    std::vector<uint8_t> result(G1_COMPRESSED_SIZE, 0);
    return result;
}

G1 Serialization::deserialize_g1_compressed(const std::vector<uint8_t>& data, size_t& offset) {
    // TODO: Implement compressed G1 deserialization
    ZK_ASSERT(offset + G1_COMPRESSED_SIZE <= data.size(), "Not enough data for compressed G1");
    offset += G1_COMPRESSED_SIZE;
    return G1();
}

std::vector<uint8_t> Serialization::serialize_g2_compressed(const G2& point) {
    // TODO: Implement compressed G2 serialization
    std::vector<uint8_t> result(G2_COMPRESSED_SIZE, 0);
    return result;
}

G2 Serialization::deserialize_g2_compressed(const std::vector<uint8_t>& data, size_t& offset) {
    // TODO: Implement compressed G2 deserialization
    ZK_ASSERT(offset + G2_COMPRESSED_SIZE <= data.size(), "Not enough data for compressed G2");
    offset += G2_COMPRESSED_SIZE;
    return G2();
}

// Integer serialization helpers
void Serialization::write_uint32(std::vector<uint8_t>& data, uint32_t value) {
    data.push_back(static_cast<uint8_t>(value & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void Serialization::write_uint64(std::vector<uint8_t>& data, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        data.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

uint32_t Serialization::read_uint32(const std::vector<uint8_t>& data, size_t& offset) {
    ZK_ASSERT(offset + 4 <= data.size(), "Not enough data for uint32");
    
    uint32_t value = data[offset] | 
                    (static_cast<uint32_t>(data[offset + 1]) << 8) |
                    (static_cast<uint32_t>(data[offset + 2]) << 16) |
                    (static_cast<uint32_t>(data[offset + 3]) << 24);
    offset += 4;
    return value;
}

uint64_t Serialization::read_uint64(const std::vector<uint8_t>& data, size_t& offset) {
    ZK_ASSERT(offset + 8 <= data.size(), "Not enough data for uint64");
    
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
    }
    offset += 8;
    return value;
}

} // namespace zkmini
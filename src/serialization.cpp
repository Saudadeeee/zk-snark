#include "zkmini/serialization.hpp"
#include "zkmini/field.hpp"
#include "zkmini/fq.hpp"
#include "zkmini/fq2.hpp"
#include "zkmini/g1.hpp"
#include "zkmini/g2.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace zkmini {

// Helper to write binary data
template<typename T>
std::vector<uint8_t> Serialization::serialize_vector(const std::vector<T>& vec) {
    std::vector<uint8_t> result;
    write_uint32(result, static_cast<uint32_t>(vec.size()));
    for (const auto& item : vec) {
        auto item_bytes = serialize_g1(item); // Assuming T is serializable
        result.insert(result.end(), item_bytes.begin(), item_bytes.end());
    }
    return result;
}

template<typename T>
std::vector<T> Serialization::deserialize_vector(const std::vector<uint8_t>& data, size_t& offset) {
    uint32_t size = read_uint32(data, offset);
    std::vector<T> result;
    result.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
        result.push_back(deserialize_g1(data, offset)); // Assuming T is deserializable
    }
    return result;
}

std::vector<uint8_t> Serialization::serialize_fr(const Fr& element) {
    std::vector<uint8_t> result(FR_SIZE, 0);
    // Simple implementation - just copy raw bytes
    const uint64_t* data_ptr = element.data.data();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            result[i * 8 + j] = (data_ptr[i] >> (j * 8)) & 0xFF;
        }
    }
    return result;
}

Fr Serialization::deserialize_fr(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + FR_SIZE > data.size()) {
        offset += FR_SIZE;
        return Fr();
    }
    
    std::array<uint64_t, 4> limbs = {0};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            limbs[i] |= static_cast<uint64_t>(data[offset + i * 8 + j]) << (j * 8);
        }
    }
    offset += FR_SIZE;
    return Fr(limbs);
}

std::vector<uint8_t> Serialization::serialize_g1(const G1& point) {
    if (point.is_zero()) {
        std::vector<uint8_t> result(G1_SIZE, 0);
        result[0] = 1; // Mark as infinity
        return result;
    }
    
    // Simple implementation - serialize as raw coordinates
    std::vector<uint8_t> result;
    result.resize(G1_SIZE);
    std::fill(result.begin(), result.end(), 0x42); // Placeholder data
    return result;
}

G1 Serialization::deserialize_g1(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + G1_SIZE > data.size()) {
        offset += G1_SIZE;
        return G1();
    }
    
    if (data[offset] == 1) {
        offset += G1_SIZE;
        return G1(); // Return point at infinity
    }
    
    offset += G1_SIZE;
    return G1(Fq(1), Fq(2)); // Placeholder - return generator
}

std::vector<uint8_t> Serialization::serialize_g2(const G2& point) {
    if (point.is_zero()) {
        std::vector<uint8_t> result(G2_SIZE, 0);
        result[0] = 1; // Mark as infinity
        return result;
    }
    
    std::vector<uint8_t> result;
    result.resize(G2_SIZE);
    std::fill(result.begin(), result.end(), 0x43); // Placeholder data
    return result;
}

G2 Serialization::deserialize_g2(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + G2_SIZE > data.size()) {
        offset += G2_SIZE;
        return G2();
    }
    
    if (data[offset] == 1) {
        offset += G2_SIZE;
        return G2(); // Return point at infinity
    }
    
    offset += G2_SIZE;
    return G2(Fq2(Fq(1), Fq(0)), Fq2(Fq(2), Fq(0))); // Placeholder
}

void Serialization::write_file(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }
}

std::vector<uint8_t> Serialization::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    }
    return {};
}

std::string Serialization::fr_to_json(const Fr& element) {
    return "\"0x123456789abcdef\""; // Placeholder
}

Fr Serialization::fr_from_json(const std::string& json_str) {
    (void)json_str; // Silence unused parameter warning
    return Fr();
}

std::string Serialization::g1_to_json(const G1& point) {
    if (point.is_zero()) {
        return "{\"infinity\":true}";
    }
    return "{\"x\":\"0x1\",\"y\":\"0x2\"}";
}

G1 Serialization::g1_from_json(const std::string& json_str) {
    (void)json_str;
    return G1();
}

std::string Serialization::g2_to_json(const G2& point) {
    if (point.is_zero()) {
        return "{\"infinity\":true}";
    }
    return "{\"x\":[\"0x1\",\"0x2\"],\"y\":[\"0x3\",\"0x4\"]}";
}

G2 Serialization::g2_from_json(const std::string& json_str) {
    (void)json_str;
    return G2();
}

std::vector<uint8_t> Serialization::serialize_g1_compressed(const G1& point) {
    if (point.is_zero()) {
        std::vector<uint8_t> result(G1_COMPRESSED_SIZE, 0);
        result[0] = 0x40; // Infinity flag
        return result;
    }
    
    std::vector<uint8_t> result(G1_COMPRESSED_SIZE, 0);
    result[0] = 0x80; // Compressed flag
    // Add actual compression logic here
    return result;
}

G1 Serialization::deserialize_g1_compressed(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + G1_COMPRESSED_SIZE > data.size()) {
        offset += G1_COMPRESSED_SIZE;
        return G1();
    }
    
    uint8_t flags = data[offset];
    offset += G1_COMPRESSED_SIZE;
    
    if (flags & 0x40) {
        return G1(); // Point at infinity
    }
    
    // Add decompression logic here
    return G1(Fq(1), Fq(2));
}

std::vector<uint8_t> Serialization::serialize_g2_compressed(const G2& point) {
    if (point.is_zero()) {
        std::vector<uint8_t> result(G2_COMPRESSED_SIZE, 0);
        result[0] = 0x40; // Infinity flag
        return result;
    }
    
    std::vector<uint8_t> result(G2_COMPRESSED_SIZE, 0);
    result[0] = 0x80; // Compressed flag
    return result;
}

G2 Serialization::deserialize_g2_compressed(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + G2_COMPRESSED_SIZE > data.size()) {
        offset += G2_COMPRESSED_SIZE;
        return G2();
    }
    
    uint8_t flags = data[offset];
    offset += G2_COMPRESSED_SIZE;
    
    if (flags & 0x40) {
        return G2(); // Point at infinity
    }
    
    return G2(Fq2(Fq(1), Fq(0)), Fq2(Fq(2), Fq(0)));
}

void Serialization::write_uint32(std::vector<uint8_t>& data, uint32_t value) {
    data.push_back(value & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 24) & 0xFF);
}

void Serialization::write_uint64(std::vector<uint8_t>& data, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        data.push_back((value >> (i * 8)) & 0xFF);
    }
}

uint32_t Serialization::read_uint32(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 4 > data.size()) {
        offset += 4;
        return 0;
    }
    
    uint32_t result = data[offset] | (data[offset + 1] << 8) | 
                      (data[offset + 2] << 16) | (data[offset + 3] << 24);
    offset += 4;
    return result;
}

uint64_t Serialization::read_uint64(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 8 > data.size()) {
        offset += 8;
        return 0;
    }
    
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
    }
    offset += 8;
    return result;
}

std::vector<uint8_t> Serialization::serialize_fq(const Fq& element) {
    std::vector<uint8_t> result(32, 0);
    for (int i = 0; i < 4; ++i) {
        uint64_t limb = element.get_data(i);
        for (int j = 0; j < 8; ++j) {
            result[i * 8 + j] = (limb >> (j * 8)) & 0xFF;
        }
    }
    return result;
}

Fq Serialization::deserialize_fq(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset + 32 > data.size()) {
        offset += 32;
        return Fq();
    }
    
    uint64_t limbs[4] = {0};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            limbs[i] |= static_cast<uint64_t>(data[offset + i * 8 + j]) << (j * 8);
        }
    }
    offset += 32;
    
    // Create Fq from limbs (simplified)
    return Fq(limbs[0]);
}

std::vector<uint8_t> Serialization::serialize_fq2(const Fq2& element) {
    auto c0_bytes = serialize_fq(element.c0);
    auto c1_bytes = serialize_fq(element.c1);
    
    std::vector<uint8_t> result;
    result.insert(result.end(), c0_bytes.begin(), c0_bytes.end());
    result.insert(result.end(), c1_bytes.begin(), c1_bytes.end());
    return result;
}

Fq2 Serialization::deserialize_fq2(const std::vector<uint8_t>& data, size_t& offset) {
    Fq c0 = deserialize_fq(data, offset);
    Fq c1 = deserialize_fq(data, offset);
    return Fq2(c0, c1);
}

// Explicit template instantiations
template std::vector<uint8_t> Serialization::serialize_vector<G1>(const std::vector<G1>&);
template std::vector<G1> Serialization::deserialize_vector<G1>(const std::vector<uint8_t>&, size_t&);

} // namespace zkmini
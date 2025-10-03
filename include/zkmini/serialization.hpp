#pragma once

#include "keys.hpp"
#include "groth16.hpp"
#include <string>
#include <vector>
#include <fstream>

namespace zkmini {
class Serialization {
public:
    
    template<typename T>
    static std::vector<uint8_t> serialize_vector(const std::vector<T>& vec);
    
    template<typename T>
    static std::vector<T> deserialize_vector(const std::vector<uint8_t>& data, size_t& offset);
    
    
    static std::vector<uint8_t> serialize_fr(const Fr& element);
    static Fr deserialize_fr(const std::vector<uint8_t>& data, size_t& offset);
    
    
    static std::vector<uint8_t> serialize_g1(const G1& point);
    static G1 deserialize_g1(const std::vector<uint8_t>& data, size_t& offset);
    
    static std::vector<uint8_t> serialize_g2(const G2& point);
    static G2 deserialize_g2(const std::vector<uint8_t>& data, size_t& offset);
    
    
    static void write_file(const std::string& filename, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> read_file(const std::string& filename);
    
    
    static std::string fr_to_json(const Fr& element);
    static Fr fr_from_json(const std::string& json_str);
    
    static std::string g1_to_json(const G1& point);
    static G1 g1_from_json(const std::string& json_str);
    
    static std::string g2_to_json(const G2& point);
    static G2 g2_from_json(const std::string& json_str);
    
    
    static std::vector<uint8_t> serialize_g1_compressed(const G1& point);
    static G1 deserialize_g1_compressed(const std::vector<uint8_t>& data, size_t& offset);
    
    static std::vector<uint8_t> serialize_g2_compressed(const G2& point);
    static G2 deserialize_g2_compressed(const std::vector<uint8_t>& data, size_t& offset);

    
    static void write_uint64(std::vector<uint8_t>& data, uint64_t value);
    static void write_uint32(std::vector<uint8_t>& data, uint32_t value);
    static uint64_t read_uint64(const std::vector<uint8_t>& data, size_t& offset);
    static uint32_t read_uint32(const std::vector<uint8_t>& data, size_t& offset);
    
    
    static std::vector<uint8_t> serialize_fq(const Fq& element);
    static Fq deserialize_fq(const std::vector<uint8_t>& data, size_t& offset);
    static std::vector<uint8_t> serialize_fq2(const Fq2& element);
    static Fq2 deserialize_fq2(const std::vector<uint8_t>& data, size_t& offset);
    
    
    static constexpr size_t FR_SIZE = 32;  
    static constexpr size_t G1_SIZE = 64;  
    static constexpr size_t G2_SIZE = 128; 
    static constexpr size_t G1_COMPRESSED_SIZE = 33; 
    static constexpr size_t G2_COMPRESSED_SIZE = 65; 
};

}
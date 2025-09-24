#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>

#define FIELD_SIZE 256

namespace zkmini {

class Fr;

namespace bn254_fr {
    constexpr uint64_t MODULUS_DEV = 0x1FFFFFFFFFFFFFFFULL;
    
    constexpr std::array<uint64_t, 4> MODULUS_BN254 = {
        0x43e1f593f0000001ULL,
        0x2833e84879b97091ULL, 
        0xb85045b68181585dULL,
        0x30644e72e131a029ULL
    };
    
    constexpr std::array<uint64_t, 4> R_BN254 = {0};
    constexpr std::array<uint64_t, 4> R2_BN254 = {0};
    constexpr uint64_t INV_BN254 = 0;
}

class Fr {
public:
    union {
        uint64_t val;
        std::array<uint64_t, 4> data;
    };
    
    static constexpr bool USE_64BIT_DEV = false;
    static constexpr uint64_t MODULUS = bn254_fr::MODULUS_DEV;

    // Constructors
    Fr();
    explicit Fr(uint64_t value);
    explicit Fr(const std::array<uint64_t, 4>& limbs);
    
    // Static factory methods
    static Fr zero();
    static Fr one();
    static Fr from_uint64(uint64_t value);
    static Fr random();
    
    Fr operator+(const Fr& other) const;
    Fr operator-(const Fr& other) const;
    Fr operator*(const Fr& other) const;
    Fr operator/(const Fr& other) const;
    Fr& operator+=(const Fr& other);
    Fr& operator-=(const Fr& other);
    Fr& operator*=(const Fr& other);
    Fr& operator/=(const Fr& other);
    
    Fr operator-() const;
    Fr neg() const;
    
    // Comparison operations
    bool operator==(const Fr& other) const;
    bool operator!=(const Fr& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    // Advanced operations
    Fr square() const;
    Fr pow(uint64_t exponent) const;
    Fr pow(const Fr& exponent) const;
    Fr inverse() const;
    
    uint64_t to_uint64() const;
    std::vector<uint8_t> to_bytes() const;
    static Fr from_bytes(const std::vector<uint8_t>& bytes);
    std::string to_hex() const;
    static Fr from_hex(const std::string& hex);
    
    // Utilities for constant-time operations
    static Fr conditional_select(bool condition, const Fr& a, const Fr& b);
    
    std::string to_string() const;
    bool is_valid() const;

private:
    void reduce();
    static uint64_t add_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t mod);
    static uint64_t inv_mod(uint64_t a, uint64_t mod);
    
    static std::array<uint64_t, 4> add_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);
    static std::array<uint64_t, 4> sub_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);
    static std::array<uint64_t, 4> neg_256(const std::array<uint64_t, 4>& a);
    static std::array<uint64_t, 4> mul_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);
    static std::array<uint64_t, 4> pow_256(const std::array<uint64_t, 4>& base, uint64_t exp);
    static std::array<uint64_t, 4> pow_256(const std::array<uint64_t, 4>& base, const std::array<uint64_t, 4>& exp);
    static std::array<uint64_t, 4> inv_256(const std::array<uint64_t, 4>& a);
    static void reduce_256(std::array<uint64_t, 4>& a);
    static bool is_less_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);
    static bool is_zero_256(const std::array<uint64_t, 4>& a);
    
    // Helper functions for Extended Euclidean Algorithm
    static bool is_even_256(const std::array<uint64_t, 4>& a);
    static bool is_one_256(const std::array<uint64_t, 4>& a);
    static std::array<uint64_t, 4> div2_256(const std::array<uint64_t, 4>& a);
    static std::array<uint64_t, 4> sub_256_signed(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);

};

// Stream operators
std::ostream& operator<<(std::ostream& os, const Fr& fr);
std::istream& operator>>(std::istream& is, Fr& fr);

} // namespace zkmini

#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <vector>

#define FIELD_SIZE 256

namespace zkmini {

// Forward declarations
class Fr;

// Constants for BN254 scalar field
namespace bn254_fr {
    // BN254 scalar field modulus: r = 21888242871839275222246405745257275088548364400416034343698204186575808495617
    // For development phase, we'll use a smaller 64-bit prime: 2^61 - 1 = 2305843009213693951
    // This is a Mersenne prime, making arithmetic easier for debugging
    constexpr uint64_t MODULUS_DEV = 0x1FFFFFFFFFFFFFFFULL; // 2^61 - 1
    
    // BN254 modulus components (for future use)
    constexpr std::array<uint64_t, 4> MODULUS_BN254 = {
        0x43e1f593f0000001ULL,
        0x2833e84879b97091ULL, 
        0xb85045b68181585dULL,
        0x30644e72e131a029ULL
    };
    
    // Montgomery constants (will be computed when switching to BN254)
    constexpr std::array<uint64_t, 4> R_BN254 = {0}; // 2^256 mod p
    constexpr std::array<uint64_t, 4> R2_BN254 = {0}; // 2^512 mod p  
    constexpr uint64_t INV_BN254 = 0; // -p^(-1) mod 2^64
}

/**
 * Field element in the BN254 scalar field Fr
 * 
 * Design decisions:
 * - Development phase: 64-bit arithmetic with __int128 for intermediate calculations
 * - Production phase: 256-bit Montgomery arithmetic with 4x64-bit limbs
 * - Invariant: All Fr elements are always in range [0, p-1]
 * - API designed to be stable across implementation changes
 */
class Fr {
public:
    // Internal representation - will evolve from 64-bit to 256-bit
    union {
        uint64_t val;           // For 64-bit development phase
        std::array<uint64_t, 4> data; // For 256-bit BN254 phase
    };
    
    // Static configuration - switch between development and production
    static constexpr bool USE_64BIT_DEV = true;
    static constexpr uint64_t MODULUS = bn254_fr::MODULUS_DEV;

    // Constructors
    Fr();
    explicit Fr(uint64_t value);
    explicit Fr(const std::array<uint64_t, 4>& limbs);
    
    // Static factory methods
    static Fr zero();
    static Fr one();
    static Fr from_uint64(uint64_t value);
    static Fr random(); // For testing
    
    // Arithmetic operations
    Fr operator+(const Fr& other) const;
    Fr operator-(const Fr& other) const;
    Fr operator*(const Fr& other) const;
    Fr operator/(const Fr& other) const;
    Fr& operator+=(const Fr& other);
    Fr& operator-=(const Fr& other);
    Fr& operator*=(const Fr& other);
    Fr& operator/=(const Fr& other);
    
    Fr operator-() const; // Negation
    Fr neg() const; // Alternative negation
    
    // Comparison operations
    bool operator==(const Fr& other) const;
    bool operator!=(const Fr& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    // Advanced operations
    Fr square() const;
    Fr pow(uint64_t exponent) const;
    Fr pow(const Fr& exponent) const;
    Fr inverse() const; // Returns 0 if input is 0
    
    // Conversion and serialization
    uint64_t to_uint64() const; // Only valid for development phase
    std::vector<uint8_t> to_bytes() const; // Little-endian
    static Fr from_bytes(const std::vector<uint8_t>& bytes);
    std::string to_hex() const;
    static Fr from_hex(const std::string& hex);
    
    // Utilities for constant-time operations
    static Fr conditional_select(bool condition, const Fr& a, const Fr& b);
    
    // Debug utilities
    std::string to_string() const;
    bool is_valid() const; // Check invariant: value < MODULUS

private:
    // Internal arithmetic helpers
    void reduce(); // Ensure value is in [0, MODULUS)
    static uint64_t add_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t mod);
    static uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t mod);
    static uint64_t inv_mod(uint64_t a, uint64_t mod); // Extended Euclidean
    
    // Future: Montgomery arithmetic helpers
    // static std::array<uint64_t, 4> montgomery_mul(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b);
    // static std::array<uint64_t, 4> montgomery_reduce(const std::array<uint64_t, 8>& t);
};

// Stream operators
std::ostream& operator<<(std::ostream& os, const Fr& fr);
std::istream& operator>>(std::istream& is, Fr& fr);

} // namespace zkmini

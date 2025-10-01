#pragma once

#include "g1.hpp"
#include "g2.hpp"
#include "qap.hpp"
#include <vector>
#include <string>

namespace zkmini {

// Groth16 Proving Key
struct ProvingKey {
    // CRS elements
    G1 alpha_g1;    // [α]₁
    G1 beta_g1;     // [β]₁  
    G2 beta_g2;     // [β]₂
    G1 delta_g1;    // [δ]₁
    G2 delta_g2;    // [δ]₂
    
    // QAP polynomial evaluations
    std::vector<G1> A_query_g1;    // [A_i(τ)]₁
    std::vector<G2> B_query_g2;    // [B_i(τ)]₂
    std::vector<G1> B_query_g1;    // [B_i(τ)]₁ (for π_C)
    std::vector<G1> K_query_g1;    // [(βA_i(τ)+αB_i(τ)+C_i(τ))/δ]₁ for private vars
    std::vector<G1> H_query_g1;    // [τ^k·Z(τ)/δ]₁
    
    size_t num_variables;
    size_t num_public;
    size_t degree;
    
    ProvingKey();
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static ProvingKey deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static ProvingKey load_from_file(const std::string& filename);
};

// Groth16 Verifying Key
struct VerifyingKey {
    // CRS elements
    G1 alpha_g1;    // [α]₁
    G2 beta_g2;     // [β]₂
    G2 gamma_g2;    // [γ]₂
    G2 delta_g2;    // [δ]₂
    
    // Input coefficients for public variables
    std::vector<G1> IC_g1;  // [(βA_i(τ)+αB_i(τ)+C_i(τ))/γ]₁ for public vars
    
    size_t num_public;
    
    VerifyingKey();
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static VerifyingKey deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static VerifyingKey load_from_file(const std::string& filename);
};

// Common Reference String - contains both proving and verifying keys
struct CRS {
    ProvingKey pk;
    VerifyingKey vk;
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static CRS deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static CRS load_from_file(const std::string& filename);
    
    // Split into separate files
    void save_keys(const std::string& pk_file, const std::string& vk_file) const;
    static CRS load_keys(const std::string& pk_file, const std::string& vk_file);
};

} // namespace zkmini
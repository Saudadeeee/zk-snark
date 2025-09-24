#pragma once

#include "g1.hpp"
#include "g2.hpp"
#include "qap.hpp"
#include <vector>
#include <string>

namespace zkmini {

// Placeholder for Groth16 keys and CRS
// TODO: Implement key generation and serialization
struct ProvingKey {
    // CRS elements for prover
    G1 alpha_g1;
    G1 beta_g1;
    G2 beta_g2;
    G1 delta_g1;
    G2 delta_g2;
    
    // Encoded QAP polynomials
    std::vector<G1> A_g1;    // [A_i(τ)]_1 for i = 0..num_variables-1
    std::vector<G1> B_g1;    // [B_i(τ)]_1 for i = 0..num_variables-1
    std::vector<G2> B_g2;    // [B_i(τ)]_2 for i = 0..num_variables-1
    
    // Lagrange basis for private inputs
    std::vector<G1> L_g1;    // [(β*A_i(τ) + α*B_i(τ) + C_i(τ))/δ]_1 for private i
    
    // Powers of τ for H polynomial
    std::vector<G1> H_g1;    // [τ^i]_1 for i = 0..degree-1
    
    size_t num_variables;
    size_t num_public;
    size_t degree;
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static ProvingKey deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static ProvingKey load_from_file(const std::string& filename);
};

struct VerifyingKey {
    // CRS elements for verifier
    G1 alpha_g1;
    G2 beta_g2;
    G2 gamma_g2;
    G2 delta_g2;
    
    // Encoded public input polynomials
    std::vector<G1> gamma_abc_g1; // [(β*A_i(τ) + α*B_i(τ) + C_i(τ))/γ]_1 for i = 0..num_public
    
    size_t num_public;
    
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
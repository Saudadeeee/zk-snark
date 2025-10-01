#pragma once

#include "keys.hpp"
#include "qap.hpp"
#include "pairing.hpp"
#include "r1cs.hpp"
#include "msm.hpp"
#include <vector>

namespace zkmini {

// Groth16 Proof structure
struct Proof {
    G1 A; // π_A
    G2 B; // π_B  
    G1 C; // π_C
    
    Proof();
    Proof(const G1& A, const G2& B, const G1& C);
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static Proof deserialize(const std::vector<uint8_t>& data);
    
    void save_to_file(const std::string& filename) const;
    static Proof load_from_file(const std::string& filename);
    
    // JSON format
    std::string to_json() const;
    static Proof from_json(const std::string& json_str);
};

// Groth16 Protocol Implementation
class Groth16 {
public:
    // Main API
    static CRS setup(const R1CS& r1cs);
    static Proof prove(const ProvingKey& pk, const QAP& qap, const std::vector<Fr>& full_witness);
    static bool verify(const VerifyingKey& vk, const std::vector<Fr>& public_inputs, const Proof& proof);
    
    // End-to-end test
    static bool test_circuit(const R1CS& r1cs, const std::vector<Fr>& public_inputs, const std::vector<Fr>& private_inputs);

private:
    // Setup helpers
    static void setup_crs_elements(CRS& crs, const QAP& qap, const R1CS& r1cs,
                                  const Fr& tau, const Fr& alpha, const Fr& beta, 
                                  const Fr& gamma, const Fr& delta);
    
    // Proving helpers  
    static Polynomial compute_h_polynomial(const QAP& qap, const std::vector<Fr>& full_witness);
    
    // Verification helpers
    static G1 compute_vk_ic(const VerifyingKey& vk, const std::vector<Fr>& public_inputs);
    static bool verify_pairing_equation(const VerifyingKey& vk, const G1& vk_ic, const Proof& proof);
};

} // namespace zkmini
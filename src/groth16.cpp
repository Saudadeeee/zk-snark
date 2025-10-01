#include "zkmini/groth16.hpp"
#include "zkmini/random.hpp"
#include "zkmini/utils.hpp"
#include "zkmini/serialization.hpp"
#include <fstream>

namespace zkmini {

// Proof constructors
Proof::Proof() : A(), B(), C() {}
Proof::Proof(const G1& A, const G2& B, const G1& C) : A(A), B(B), C(C) {}

// Proof serialization
std::vector<uint8_t> Proof::serialize() const {
    std::vector<uint8_t> result;
    
    // Serialize A (G1)
    auto A_data = Serialization::serialize_g1(A);
    result.insert(result.end(), A_data.begin(), A_data.end());
    
    // Serialize B (G2)
    auto B_data = Serialization::serialize_g2(B);
    result.insert(result.end(), B_data.begin(), B_data.end());
    
    // Serialize C (G1)
    auto C_data = Serialization::serialize_g1(C);
    result.insert(result.end(), C_data.begin(), C_data.end());
    
    return result;
}

Proof Proof::deserialize(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    
    // Deserialize A (G1)
    G1 A = Serialization::deserialize_g1(data, offset);
    
    // Deserialize B (G2)
    G2 B = Serialization::deserialize_g2(data, offset);
    
    // Deserialize C (G1)
    G1 C = Serialization::deserialize_g1(data, offset);
    
    return Proof(A, B, C);
}

void Proof::save_to_file(const std::string& filename) const {
    auto data = serialize();
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

Proof Proof::load_from_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    return deserialize(data);
}

std::string Proof::to_json() const {
    return "{\"A\":" + Serialization::g1_to_json(A) + 
           ",\"B\":" + Serialization::g2_to_json(B) + 
           ",\"C\":" + Serialization::g1_to_json(C) + "}";
}

Proof Proof::from_json(const std::string& json_str) {
    // TODO: Parse JSON string properly without nlohmann dependency
    (void)json_str;
    return Proof();
}

// Main Groth16 setup: R1CS -> (ProvingKey, VerifyingKey)
CRS Groth16::setup(const R1CS& r1cs) {
    ZK_TIMER("Groth16 Setup");
    
    // Convert R1CS to QAP
    QAP qap = r1cs_to_qap(r1cs);
    
    // Sample toxic waste
    Fr tau = random_fr();
    Fr alpha = random_fr();  
    Fr beta = random_fr();
    Fr gamma = random_fr();
    Fr delta = random_fr();
    
    CRS crs;
    setup_crs_elements(crs, qap, r1cs, tau, alpha, beta, gamma, delta);
    
    return crs;
}

// Groth16 proving: witness -> proof
Proof Groth16::prove(const ProvingKey& pk, const QAP& qap, const std::vector<Fr>& full_witness) {
    ZK_TIMER("Groth16 Prove");
    
    ZK_ASSERT(full_witness.size() == pk.num_variables, "Wrong witness size");
    ZK_ASSERT(full_witness[0] == Fr(1), "Witness[0] must be 1");
    
    // Random values for zero-knowledge
    Fr r = random_fr();
    Fr s = random_fr();
    
    // Compute A(τ), B(τ) via MSM on queries
    G1 A_tau = MSM::msm_g1(full_witness, pk.A_query_g1);
    G2 B_tau_g2 = MSM::msm_g2(full_witness, pk.B_query_g2);
    G1 B_tau_g1 = MSM::msm_g1(full_witness, pk.B_query_g1);
    
    // Compute H polynomial and evaluate
    Polynomial H_poly = compute_h_polynomial(qap, full_witness);
    std::vector<Fr> h_coeffs = H_poly.coefficients();
    h_coeffs.resize(pk.degree, Fr(0)); // Pad to degree
    
    G1 H_tau = MSM::msm_g1(h_coeffs, pk.H_query_g1);
    
    // Compute K contribution (private variables only)
    std::vector<Fr> private_witness;
    for (size_t i = pk.num_public + 1; i < full_witness.size(); ++i) {
        private_witness.push_back(full_witness[i]);
    }
    G1 K_contribution = MSM::msm_g1(private_witness, pk.K_query_g1);
    
    // Assemble proof components
    Proof proof;
    proof.A = pk.alpha_g1 + A_tau + (pk.delta_g1 * r);
    proof.B = pk.beta_g2 + B_tau_g2 + (pk.delta_g2 * s);  
    proof.C = H_tau + K_contribution + (A_tau * s) + (B_tau_g1 * r) + (pk.delta_g1 * (r * s));
    
    return proof;
}

// Groth16 verification: public_inputs + proof -> bool
bool Groth16::verify(const VerifyingKey& vk, const std::vector<Fr>& public_inputs, const Proof& proof) {
    ZK_TIMER("Groth16 Verify");
    
    ZK_ASSERT(public_inputs.size() == vk.num_public, "Wrong number of public inputs");
    
    // Compute input commitment
    G1 vk_ic = compute_vk_ic(vk, public_inputs);
    
    // Verify pairing equation
    return verify_pairing_equation(vk, vk_ic, proof);
}

// End-to-end test
bool Groth16::test_circuit(const R1CS& r1cs, 
                          const std::vector<Fr>& public_inputs,
                          const std::vector<Fr>& private_inputs) {
    // Generate full witness
    std::vector<Fr> full_witness = r1cs.generate_full_assignment(public_inputs, private_inputs);
    
    // Debug: print witness
    std::cout << "Generated witness: [";
    for (size_t i = 0; i < full_witness.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << full_witness[i].to_string();
    }
    std::cout << "]" << std::endl;
    
    // Setup
    CRS crs = setup(r1cs);
    QAP qap = r1cs_to_qap(r1cs);
    
    // Prove
    Proof proof = prove(crs.pk, qap, full_witness);
    
    // Verify
    return verify(crs.vk, public_inputs, proof);
}

// Setup CRS elements from QAP and toxic waste
void Groth16::setup_crs_elements(CRS& crs, const QAP& qap, const R1CS& r1cs,
                                const Fr& tau, const Fr& alpha, const Fr& beta, 
                                const Fr& gamma, const Fr& delta) {
    size_t n = qap.n;
    size_t m = qap.m;
    
    // Basic setup
    crs.pk.num_variables = n;
    crs.pk.num_public = r1cs.public_inputs().size(); 
    crs.pk.degree = m;
    crs.vk.num_public = crs.pk.num_public;
    
    // Setup CRS elements for proving key
    crs.pk.alpha_g1 = G1::generator() * alpha;
    crs.pk.beta_g1 = G1::generator() * beta;
    crs.pk.beta_g2 = G2::generator() * beta;
    crs.pk.delta_g1 = G1::generator() * delta;
    crs.pk.delta_g2 = G2::generator() * delta;
    
    crs.vk.alpha_g1 = crs.pk.alpha_g1;
    crs.vk.beta_g2 = G2::generator() * beta;
    crs.vk.gamma_g2 = G2::generator() * gamma;
    crs.vk.delta_g2 = crs.pk.delta_g2;
    
    // Resize query vectors
    crs.pk.A_query_g1.resize(n);
    crs.pk.B_query_g2.resize(n); 
    crs.pk.B_query_g1.resize(n);
    crs.pk.H_query_g1.resize(m);
    
    // Evaluate polynomials at τ
    for (size_t i = 0; i < n; ++i) {
        Fr a_i = qap.A_basis[i].evaluate(tau);
        Fr b_i = qap.B_basis[i].evaluate(tau);
        Fr c_i = qap.C_basis[i].evaluate(tau);
        
        crs.pk.A_query_g1[i] = G1::generator() * a_i;
        crs.pk.B_query_g2[i] = G2::generator() * b_i;
        crs.pk.B_query_g1[i] = G1::generator() * b_i;
        
        // K query for private variables only
        bool is_public = (i == 0); // x0 = 1 is always public
        for (size_t pub_idx : r1cs.public_inputs()) {
            if (i == pub_idx) {
                is_public = true;
                break;
            }
        }
        
        if (!is_public) {
            Fr k_val = (beta * a_i + alpha * b_i + c_i) / delta;
            crs.pk.K_query_g1.push_back(G1::generator() * k_val);
        }
    }
    
    // H query: [τ^k * Z(τ) / δ]₁
    Fr z_tau = qap.Z.evaluate(tau);
    Fr tau_power = Fr(1);
    for (size_t k = 0; k < m; ++k) {
        Fr h_k = tau_power * z_tau / delta;
        crs.pk.H_query_g1[k] = G1::generator() * h_k;
        tau_power = tau_power * tau;
    }
    
    // IC for verifying key (public variables + constant)
    crs.vk.IC_g1.resize(crs.vk.num_public + 1);
    
    // IC[0] for constant x0 = 1
    Fr a_0 = qap.A_basis[0].evaluate(tau);
    Fr b_0 = qap.B_basis[0].evaluate(tau); 
    Fr c_0 = qap.C_basis[0].evaluate(tau);
    crs.vk.IC_g1[0] = G1::generator() * ((beta * a_0 + alpha * b_0 + c_0) / gamma);
    
    // IC[j] for public inputs
    for (size_t j = 0; j < r1cs.public_inputs().size(); ++j) {
        size_t pub_idx = r1cs.public_inputs()[j];
        Fr a_pub = qap.A_basis[pub_idx].evaluate(tau);
        Fr b_pub = qap.B_basis[pub_idx].evaluate(tau);
        Fr c_pub = qap.C_basis[pub_idx].evaluate(tau);
        crs.vk.IC_g1[j + 1] = G1::generator() * ((beta * a_pub + alpha * b_pub + c_pub) / gamma);
    }
}

// Compute H polynomial from witness
Polynomial Groth16::compute_h_polynomial(const QAP& qap, const std::vector<Fr>& full_witness) {
    // Assemble witness polynomials
    Polynomial A_poly = assemble_A(qap, full_witness);
    Polynomial B_poly = assemble_B(qap, full_witness);
    Polynomial C_poly = assemble_C(qap, full_witness);
    
    // Compute numerator: A(X) * B(X) - C(X)
    Polynomial numerator = A_poly * B_poly - C_poly;
    
    // Divide by Z(X)
    auto [quotient, remainder] = numerator.divide(qap.Z);
    
    ZK_ASSERT(remainder.is_zero(), "H polynomial division must be exact");
    return quotient;
}

// Compute input commitment for verification
G1 Groth16::compute_vk_ic(const VerifyingKey& vk, const std::vector<Fr>& public_inputs) {
    ZK_ASSERT(public_inputs.size() == vk.num_public, "Wrong number of public inputs");
    ZK_ASSERT(vk.IC_g1.size() == vk.num_public + 1, "IC size mismatch");
    
    // Start with IC[0] (constant term)
    G1 result = vk.IC_g1[0];
    
    // Add public input contributions
    for (size_t i = 0; i < public_inputs.size(); ++i) {
        result = result + (vk.IC_g1[i + 1] * public_inputs[i]);
    }
    
    return result;
}

// Verify pairing equation: e(π_A, π_B) = e(α, β) * e(IC, γ) * e(π_C, δ)
bool Groth16::verify_pairing_equation(const VerifyingKey& vk, const G1& vk_ic, const Proof& proof) {
    // Use pairing check for efficiency:
    // e(π_A, π_B) * e(-α, β) * e(-IC, γ) * e(-π_C, δ) = 1
    
    std::vector<G1> P_vec = {
        proof.A,
        vk.alpha_g1.negate(),
        vk_ic.negate(), 
        proof.C.negate()
    };
    
    std::vector<G2> Q_vec = {
        proof.B,
        vk.beta_g2,
        vk.gamma_g2,
        vk.delta_g2
    };
    
    return Pairing::pairing_check(P_vec, Q_vec);
}

} // namespace zkmini
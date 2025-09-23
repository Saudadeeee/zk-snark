#pragma once

#include ".r1cs.hpp"
#include ".polynomial.hpp"
#include ".fft.hpp"
#include <vector>

namespace zkmini {

// Placeholder for QAP (Quadratic Arithmetic Program) conversion
// TODO: Implement R1CS to QAP transformation
class QAP {
public:
    size_t num_variables;
    size_t num_public;
    size_t degree;        // Degree of polynomials = number of constraints
    
    // Lagrange polynomials A_i(X), B_i(X), C_i(X) for each variable i
    std::vector<Polynomial> A; // A[i] = A_i(X)
    std::vector<Polynomial> B; // B[i] = B_i(X) 
    std::vector<Polynomial> C; // C[i] = C_i(X)
    
    // Target polynomial Z(X) = (X - r_1)(X - r_2)...(X - r_m)
    Polynomial Z;
    
    // Evaluation domain (roots r_1, r_2, ..., r_m)
    std::vector<Fr> domain;
    
    QAP();
    
    // Convert R1CS to QAP
    static QAP from_r1cs(const R1CS& r1cs);
    
    // Evaluate QAP at a point: compute A(X), B(X), C(X) and H(X)
    struct QAPEvaluation {
        Fr A_val, B_val, C_val;
        Fr H_val;  // H(X) = (A(X) * B(X) - C(X)) / Z(X)
    };
    
    QAPEvaluation evaluate_at(const Fr& x, 
                             const std::vector<Fr>& public_inputs,
                             const std::vector<Fr>& private_inputs) const;
    
    // Compute witness polynomial H(X) = (A(X) * B(X) - C(X)) / Z(X)
    Polynomial compute_h_polynomial(const std::vector<Fr>& public_inputs,
                                   const std::vector<Fr>& private_inputs) const;
    
    // Check QAP satisfiability: A(X) * B(X) - C(X) = H(X) * Z(X)
    bool is_satisfied(const std::vector<Fr>& public_inputs,
                     const std::vector<Fr>& private_inputs) const;
    
    // Get full assignment vector z = [1, public, private]
    std::vector<Fr> generate_full_assignment(const std::vector<Fr>& public_inputs,
                                           const std::vector<Fr>& private_inputs) const;

private:
    // Convert constraint matrix to Lagrange polynomials
    std::vector<Polynomial> matrix_to_polynomials(
        const std::vector<std::vector<std::pair<size_t, Fr>>>& matrix,
        const std::vector<Fr>& domain) const;
    
    // Lagrange interpolation for a single variable across all constraints
    Polynomial interpolate_variable(const std::vector<Fr>& values,
                                   const std::vector<Fr>& domain) const;
    
    // Compute A(X), B(X), C(X) polynomials from assignment
    std::tuple<Polynomial, Polynomial, Polynomial> 
    compute_abc_polynomials(const std::vector<Fr>& assignment) const;
};

} // namespace zkmini
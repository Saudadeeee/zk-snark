#pragma once

#include "r1cs.hpp"
#include "polynomial.hpp"
#include "fft.hpp"
#include <vector>

namespace zkmini {

/**
 * QAP (Quadratic Arithmetic Program) representation
 * 
 * Converts R1CS constraints into polynomial form:
 * - From m constraints and n variables (including x₀ = 1)
 * - Domain S = {1, 2, ..., m}
 * - For each variable i: basis polynomials Aᵢ(X), Bᵢ(X), Cᵢ(X)
 * - Vanishing polynomial Z(X) = ∏(X - k) for k ∈ S
 * 
 * Constraint satisfaction: A(X)·B(X) - C(X) = H(X)·Z(X)
 * where A(X) = Σ xᵢ·Aᵢ(X), B(X) = Σ xᵢ·Bᵢ(X), C(X) = Σ xᵢ·Cᵢ(X)
 */
struct QAP {
    size_t m;  // Number of constraints (domain size)
    size_t n;  // Number of variables (including x₀ = 1)
    
    // Basis polynomials for each variable
    std::vector<Polynomial> A_basis;  // A_basis[i] = Aᵢ(X)
    std::vector<Polynomial> B_basis;  // B_basis[i] = Bᵢ(X)
    std::vector<Polynomial> C_basis;  // C_basis[i] = Cᵢ(X)
    
    // Vanishing polynomial Z(X) = (X-1)(X-2)...(X-m)
    Polynomial Z;
    
    // Domain points [1, 2, ..., m] (optional for debugging)
    std::vector<Fr> domain_points;
    
    // Constructors
    QAP() : m(0), n(0) {}
    QAP(size_t m_constraints, size_t n_variables) 
        : m(m_constraints), n(n_variables) {
        A_basis.resize(n);
        B_basis.resize(n);
        C_basis.resize(n);
    }
    
    // Invariant checks
    bool is_valid() const {
        return A_basis.size() == n && 
               B_basis.size() == n && 
               C_basis.size() == n &&
               (domain_points.empty() || domain_points.size() == m);
    }
};

// Core QAP operations

/**
 * Convert R1CS to QAP
 * @param r R1CS constraint system
 * @return QAP with basis polynomials and vanishing polynomial
 */
QAP r1cs_to_qap(const R1CS& r);

/**
 * Assemble witness polynomials A(X), B(X), C(X)
 * @param q QAP instance
 * @param x Witness vector [1, x₁, x₂, ..., xₙ₋₁] with x[0] = 1
 * @return A(X) = Σ x[i] * A_basis[i]
 */
Polynomial assemble_A(const QAP& q, const std::vector<Fr>& x);
Polynomial assemble_B(const QAP& q, const std::vector<Fr>& x);
Polynomial assemble_C(const QAP& q, const std::vector<Fr>& x);

/**
 * Check polynomial divisibility
 * @param N Numerator polynomial
 * @param D Denominator polynomial
 * @return true if N is divisible by D (remainder is zero)
 */
bool divides(const Polynomial& N, const Polynomial& D);

/**
 * Compute quotient polynomial H(X) = (A(X)·B(X) - C(X)) / Z(X)
 * @param A, B, C Witness polynomials
 * @param Z Vanishing polynomial
 * @return H(X) quotient polynomial
 * @throws If division is not exact (remainder != 0)
 */
Polynomial compute_H(const Polynomial& A, const Polynomial& B, 
                    const Polynomial& C, const Polynomial& Z);

/**
 * Check QAP constraint satisfaction
 * @param q QAP instance
 * @param x Witness vector with x[0] = 1
 * @return true if witness satisfies all QAP constraints
 */
bool qap_check(const QAP& q, const std::vector<Fr>& x);

/**
 * Get numerator and denominator for debugging
 * @param q QAP instance
 * @param x Witness vector
 * @return (A*B - C, Z) pair for analysis
 */
std::pair<Polynomial, Polynomial> qap_num_den(const QAP& q, const std::vector<Fr>& x);

// Debug utilities

/**
 * Debug basis polynomials for variable i
 * @param q QAP instance
 * @param i Variable index
 * @return String representation of Aᵢ(X), Bᵢ(X), Cᵢ(X)
 */
std::string debug_basis(const QAP& q, size_t i);

/**
 * Debug domain points
 * @param q QAP instance
 * @return String representation of evaluation domain
 */
std::string debug_domain(const QAP& q);

// Legacy compatibility for existing code
class QAPLegacy {
public:
    size_t num_variables;
    size_t num_public;
    size_t degree;
    
    std::vector<Polynomial> A;
    std::vector<Polynomial> B;
    std::vector<Polynomial> C;
    Polynomial Z;
    std::vector<Fr> domain;
    
    QAPLegacy();
    static QAPLegacy from_r1cs(const R1CS& r1cs);
    
    struct QAPEvaluation {
        Fr A_val, B_val, C_val;
        Fr H_val;
    };
    
    QAPEvaluation evaluate_at(const Fr& x, 
                             const std::vector<Fr>& public_inputs,
                             const std::vector<Fr>& private_inputs) const;
    
    Polynomial compute_h_polynomial(const std::vector<Fr>& public_inputs,
                                   const std::vector<Fr>& private_inputs) const;
    
    bool is_satisfied(const std::vector<Fr>& public_inputs,
                     const std::vector<Fr>& private_inputs) const;
    
    std::vector<Fr> generate_full_assignment(const std::vector<Fr>& public_inputs,
                                           const std::vector<Fr>& private_inputs) const;

private:
    std::vector<Polynomial> matrix_to_polynomials_new(
        const R1CS& r1cs,
        const std::vector<LinearCombination>& matrix,
        const std::vector<Fr>& domain) const;
    
    Polynomial interpolate_variable(const std::vector<Fr>& values,
                                   const std::vector<Fr>& domain) const;
    
    std::tuple<Polynomial, Polynomial, Polynomial> 
    compute_abc_polynomials(const std::vector<Fr>& assignment) const;
};

} 
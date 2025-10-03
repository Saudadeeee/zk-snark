#pragma once

#include "r1cs.hpp"
#include "polynomial.hpp"
#include "fft.hpp"
#include <vector>

namespace zkmini {

struct QAP {
    size_t m;  
    size_t n;  
    
    
    std::vector<Polynomial> A_basis;  
    std::vector<Polynomial> B_basis;  
    std::vector<Polynomial> C_basis;  
    
    
    Polynomial Z;
    
    
    std::vector<Fr> domain_points;
    
    
    QAP() : m(0), n(0) {}
    QAP(size_t m_constraints, size_t n_variables) 
        : m(m_constraints), n(n_variables) {
        A_basis.resize(n);
        B_basis.resize(n);
        C_basis.resize(n);
    }
    
    
    bool is_valid() const {
        return A_basis.size() == n && 
               B_basis.size() == n && 
               C_basis.size() == n &&
               (domain_points.empty() || domain_points.size() == m);
    }
};

QAP r1cs_to_qap(const R1CS& r);

Polynomial assemble_A(const QAP& q, const std::vector<Fr>& x);
Polynomial assemble_B(const QAP& q, const std::vector<Fr>& x);
Polynomial assemble_C(const QAP& q, const std::vector<Fr>& x);

bool divides(const Polynomial& N, const Polynomial& D);

Polynomial compute_H(const Polynomial& A, const Polynomial& B, 
                    const Polynomial& C, const Polynomial& Z);

bool qap_check(const QAP& q, const std::vector<Fr>& x);

std::pair<Polynomial, Polynomial> qap_num_den(const QAP& q, const std::vector<Fr>& x);

std::string debug_basis(const QAP& q, size_t i);

std::string debug_domain(const QAP& q);
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
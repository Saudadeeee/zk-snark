#include "zkmini/qap.hpp"
#include "zkmini/utils.hpp"

namespace zkmini {

QAP::QAP() : num_variables(0), num_public(0), degree(0) {}

QAP QAP::from_r1cs(const R1CS& r1cs) {
    QAP qap;
    qap.num_variables = r1cs.num_variables();
    qap.num_public = r1cs.public_inputs().size();
    qap.degree = r1cs.num_constraints();
    
    // Generate evaluation domain (roots of unity)
    // TODO: Use proper root of unity for the field
    qap.domain.clear();
    for (size_t i = 0; i < r1cs.num_constraints(); ++i) {
        qap.domain.push_back(Fr(i + 1)); // Simplified: use 1, 2, 3, ...
    }
    
    // Convert constraint matrices to Lagrange polynomials using new API
    qap.A = qap.matrix_to_polynomials_new(r1cs, r1cs.A, qap.domain);
    qap.B = qap.matrix_to_polynomials_new(r1cs, r1cs.B, qap.domain);
    qap.C = qap.matrix_to_polynomials_new(r1cs, r1cs.C, qap.domain);
    
    // Compute vanishing polynomial Z(X)
    qap.Z = Polynomial::vanishing_polynomial(qap.domain);
    
    return qap;
}

QAP::QAPEvaluation QAP::evaluate_at(const Fr& x, 
                                   const std::vector<Fr>& public_inputs,
                                   const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> assignment = generate_full_assignment(public_inputs, private_inputs);
    
    QAPEvaluation eval;
    eval.A_val = Fr();
    eval.B_val = Fr();
    eval.C_val = Fr();
    
    // Compute A(x) = Σ a_i * A_i(x)
    for (size_t i = 0; i < num_variables && i < A.size(); ++i) {
        eval.A_val = eval.A_val + (assignment[i] * A[i].evaluate(x));
    }
    
    // Compute B(x) = Σ a_i * B_i(x)
    for (size_t i = 0; i < num_variables && i < B.size(); ++i) {
        eval.B_val = eval.B_val + (assignment[i] * B[i].evaluate(x));
    }
    
    // Compute C(x) = Σ a_i * C_i(x)
    for (size_t i = 0; i < num_variables && i < C.size(); ++i) {
        eval.C_val = eval.C_val + (assignment[i] * C[i].evaluate(x));
    }
    
    // Compute H(x) = (A(x) * B(x) - C(x)) / Z(x)
    Fr numerator = eval.A_val * eval.B_val - eval.C_val;
    Fr z_val = Z.evaluate(x);
    eval.H_val = numerator / z_val; // This should be exact if constraint is satisfied
    
    return eval;
}

Polynomial QAP::compute_h_polynomial(const std::vector<Fr>& public_inputs,
                                    const std::vector<Fr>& private_inputs) const {
    auto [A_poly, B_poly, C_poly] = compute_abc_polynomials(
        generate_full_assignment(public_inputs, private_inputs)
    );
    
    // H(X) = (A(X) * B(X) - C(X)) / Z(X)
    Polynomial numerator = A_poly * B_poly - C_poly;
    auto [quotient, remainder] = numerator.divide(Z);
    
    ZK_ASSERT(remainder.is_zero(), "H polynomial division should be exact");
    return quotient;
}

bool QAP::is_satisfied(const std::vector<Fr>& public_inputs,
                      const std::vector<Fr>& private_inputs) const {
    // Check if A(X) * B(X) - C(X) = H(X) * Z(X) for all X in domain
    for (const Fr& x : domain) {
        QAPEvaluation eval = evaluate_at(x, public_inputs, private_inputs);
        Fr lhs = eval.A_val * eval.B_val - eval.C_val;
        Fr rhs = eval.H_val * Z.evaluate(x);
        if (!(lhs == rhs)) {
            return false;
        }
    }
    return true;
}

std::vector<Fr> QAP::generate_full_assignment(const std::vector<Fr>& public_inputs,
                                            const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> full_assignment;
    full_assignment.reserve(num_variables);
    
    // z[0] = 1 (constant)
    full_assignment.push_back(Fr(1));
    
    // z[1..num_public] = public inputs
    for (const Fr& input : public_inputs) {
        full_assignment.push_back(input);
    }
    
    // z[num_public+1..num_variables-1] = private inputs
    for (const Fr& input : private_inputs) {
        full_assignment.push_back(input);
    }
    
    return full_assignment;
}

std::vector<Polynomial> QAP::matrix_to_polynomials(
    const std::vector<std::vector<std::pair<size_t, Fr>>>& matrix,
    const std::vector<Fr>& domain) const {
    
    std::vector<Polynomial> polynomials(num_variables);
    
    for (size_t var = 0; var < num_variables; ++var) {
        std::vector<Fr> values(domain.size(), Fr());
        
        // Extract values for this variable across all constraints
        for (size_t constraint = 0; constraint < matrix.size(); ++constraint) {
            for (const auto& term : matrix[constraint]) {
                if (term.first == var) {
                    values[constraint] = term.second;
                    break;
                }
            }
        }
        
        // Interpolate polynomial for this variable
        polynomials[var] = interpolate_variable(values, domain);
    }
    
    return polynomials;
}

// New method to work with new R1CS API
std::vector<Polynomial> QAP::matrix_to_polynomials_new(
    const R1CS& r1cs,
    const std::vector<LinearCombination>& matrix,
    const std::vector<Fr>& domain) const {
    
    std::vector<Polynomial> polynomials(num_variables);
    
    for (size_t var = 0; var < num_variables; ++var) {
        // Use R1CS column_values method to extract coefficients for this variable
        std::vector<Fr> values = r1cs.column_values(matrix, var);
        
        // Interpolate polynomial for this variable
        polynomials[var] = interpolate_variable(values, domain);
    }
    
    return polynomials;
}

Polynomial QAP::interpolate_variable(const std::vector<Fr>& values,
                                   const std::vector<Fr>& domain) const {
    return Polynomial::lagrange_interpolate(domain, values);
}

std::tuple<Polynomial, Polynomial, Polynomial> 
QAP::compute_abc_polynomials(const std::vector<Fr>& assignment) const {
    Polynomial A_poly, B_poly, C_poly;
    
    // A(X) = Σ a_i * A_i(X)
    for (size_t i = 0; i < num_variables && i < A.size(); ++i) {
        A_poly = A_poly + (A[i] * assignment[i]);
    }
    
    // B(X) = Σ a_i * B_i(X)
    for (size_t i = 0; i < num_variables && i < B.size(); ++i) {
        B_poly = B_poly + (B[i] * assignment[i]);
    }
    
    // C(X) = Σ a_i * C_i(X)
    for (size_t i = 0; i < num_variables && i < C.size(); ++i) {
        C_poly = C_poly + (C[i] * assignment[i]);
    }
    
    return {A_poly, B_poly, C_poly};
}

} // namespace zkmini
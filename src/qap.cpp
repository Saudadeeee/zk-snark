#include "zkmini/qap.hpp"
#include "zkmini/utils.hpp"
#include <stdexcept>

namespace zkmini {

QAP r1cs_to_qap(const R1CS& r) {
    size_t m = r.num_constraints();
    size_t n = r.num_variables();
    
    QAP q(m, n);
    
    q.domain_points.clear();
    q.domain_points.reserve(m);
    for (size_t i = 0; i < m; ++i) {
        q.domain_points.push_back(Fr(i + 1));
    }
    
    for (size_t i = 0; i < n; ++i) {
        std::vector<Fr> vals_A = r.column_values(r.A, i);
        std::vector<Fr> vals_B = r.column_values(r.B, i);
        std::vector<Fr> vals_C = r.column_values(r.C, i);
        
        q.A_basis[i] = Polynomial::interpolate(q.domain_points, vals_A);
        q.B_basis[i] = Polynomial::interpolate(q.domain_points, vals_B);
        q.C_basis[i] = Polynomial::interpolate(q.domain_points, vals_C);
    }
    
    q.Z = Polynomial::vanishing(q.domain_points);
    
    return q;
}

Polynomial assemble_A(const QAP& q, const std::vector<Fr>& x) {
    ZK_ASSERT(x.size() == q.n, "Witness size mismatch");
    ZK_ASSERT(!x.empty() && x[0] == Fr(1), "First element must be 1");
    
    Polynomial result = Polynomial::zero();
    
    for (size_t i = 0; i < q.n; ++i) {
        if (!x[i].is_zero()) {
            Polynomial term = Polynomial::scalar_mul(q.A_basis[i], x[i]);
            Polynomial::add_inplace(result, term);
        }
    }
    
    result.normalize();
    return result;
}

Polynomial assemble_B(const QAP& q, const std::vector<Fr>& x) {
    ZK_ASSERT(x.size() == q.n, "Witness size mismatch");
    ZK_ASSERT(!x.empty() && x[0] == Fr(1), "First element must be 1");
    
    Polynomial result = Polynomial::zero();
    
    for (size_t i = 0; i < q.n; ++i) {
        if (!x[i].is_zero()) {
            Polynomial term = Polynomial::scalar_mul(q.B_basis[i], x[i]);
            Polynomial::add_inplace(result, term);
        }
    }
    
    result.normalize();
    return result;
}

Polynomial assemble_C(const QAP& q, const std::vector<Fr>& x) {
    ZK_ASSERT(x.size() == q.n, "Witness size mismatch");
    ZK_ASSERT(!x.empty() && x[0] == Fr(1), "First element must be 1");
    
    Polynomial result = Polynomial::zero();
    
    for (size_t i = 0; i < q.n; ++i) {
        if (!x[i].is_zero()) {
            Polynomial term = Polynomial::scalar_mul(q.C_basis[i], x[i]);
            Polynomial::add_inplace(result, term);
        }
    }
    
    result.normalize();
    return result;
}

bool divides(const Polynomial& N, const Polynomial& D) {
    if (D.is_zero()) {
        throw std::invalid_argument("Division by zero polynomial");
    }
    
    Polynomial Q, R;
    Polynomial::divrem(N, D, Q, R);
    return R.is_zero();
}

Polynomial compute_H(const Polynomial& A, const Polynomial& B, 
                    const Polynomial& C, const Polynomial& Z) {
    Polynomial AB = Polynomial::mul_schoolbook(A, B);
    Polynomial numerator = Polynomial::sub(AB, C);
    
    Polynomial H, remainder;
    Polynomial::divrem(numerator, Z, H, remainder);
    
    if (!remainder.is_zero()) {
        throw std::runtime_error("QAP constraint not satisfied: (A*B - C) not divisible by Z");
    }
    
    return H;
}

bool qap_check(const QAP& q, const std::vector<Fr>& x) {
    try {
        Polynomial A = assemble_A(q, x);
        Polynomial B = assemble_B(q, x);
        Polynomial C = assemble_C(q, x);
        
        Polynomial AB = Polynomial::mul_schoolbook(A, B);
        Polynomial numerator = Polynomial::sub(AB, C);
        
        return divides(numerator, q.Z);
    } catch (...) {
        return false;
    }
}

std::pair<Polynomial, Polynomial> qap_num_den(const QAP& q, const std::vector<Fr>& x) {
    Polynomial A = assemble_A(q, x);
    Polynomial B = assemble_B(q, x);
    Polynomial C = assemble_C(q, x);
    
    Polynomial AB = Polynomial::mul_schoolbook(A, B);
    Polynomial numerator = Polynomial::sub(AB, C);
    
    return {numerator, q.Z};
}

std::string debug_basis(const QAP& q, size_t i) {
    if (i >= q.n) {
        return "Invalid variable index";
    }
    
    std::string result = "Variable " + std::to_string(i) + ":\n";
    result += "  A_" + std::to_string(i) + "(X) = " + q.A_basis[i].to_string() + "\n";
    result += "  B_" + std::to_string(i) + "(X) = " + q.B_basis[i].to_string() + "\n";
    result += "  C_" + std::to_string(i) + "(X) = " + q.C_basis[i].to_string();
    
    return result;
}

std::string debug_domain(const QAP& q) {
    std::string result = "Domain points: [";
    for (size_t i = 0; i < q.domain_points.size(); ++i) {
        if (i > 0) result += ", ";
        result += q.domain_points[i].to_string();
    }
    result += "]";
    
    return result;
}

QAPLegacy::QAPLegacy() : num_variables(0), num_public(0), degree(0) {}

QAPLegacy QAPLegacy::from_r1cs(const R1CS& r1cs) {
    QAP modern_qap = r1cs_to_qap(r1cs);
    
    QAPLegacy legacy;
    legacy.num_variables = modern_qap.n;
    legacy.num_public = r1cs.public_inputs().size();
    legacy.degree = modern_qap.m;
    
    legacy.A = modern_qap.A_basis;
    legacy.B = modern_qap.B_basis;
    legacy.C = modern_qap.C_basis;
    legacy.Z = modern_qap.Z;
    legacy.domain = modern_qap.domain_points;
    
    return legacy;
}

QAPLegacy::QAPEvaluation QAPLegacy::evaluate_at(const Fr& x, 
                                   const std::vector<Fr>& public_inputs,
                                   const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> assignment = generate_full_assignment(public_inputs, private_inputs);
    
    QAPEvaluation eval;
    eval.A_val = Fr();
    eval.B_val = Fr();
    eval.C_val = Fr();
    
    for (size_t i = 0; i < num_variables && i < A.size(); ++i) {
        eval.A_val = eval.A_val + (assignment[i] * A[i].evaluate(x));
    }
    
    for (size_t i = 0; i < num_variables && i < B.size(); ++i) {
        eval.B_val = eval.B_val + (assignment[i] * B[i].evaluate(x));
    }
    
    for (size_t i = 0; i < num_variables && i < C.size(); ++i) {
        eval.C_val = eval.C_val + (assignment[i] * C[i].evaluate(x));
    }
    
    Fr numerator = eval.A_val * eval.B_val - eval.C_val;
    Fr z_val = Z.evaluate(x);
    eval.H_val = numerator / z_val;
    
    return eval;
}

Polynomial QAPLegacy::compute_h_polynomial(const std::vector<Fr>& public_inputs,
                                          const std::vector<Fr>& private_inputs) const {
    auto [A_poly, B_poly, C_poly] = compute_abc_polynomials(
        generate_full_assignment(public_inputs, private_inputs)
    );
    
    Polynomial numerator = A_poly * B_poly - C_poly;
    auto [quotient, remainder] = numerator.divide(Z);
    
    ZK_ASSERT(remainder.is_zero(), "H polynomial division should be exact");
    return quotient;
}

bool QAPLegacy::is_satisfied(const std::vector<Fr>& public_inputs,
                           const std::vector<Fr>& private_inputs) const {

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

std::vector<Fr> QAPLegacy::generate_full_assignment(const std::vector<Fr>& public_inputs,
                                                  const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> full_assignment;
    full_assignment.reserve(num_variables);
    
    full_assignment.push_back(Fr(1));
    
    for (const Fr& input : public_inputs) {
        full_assignment.push_back(input);
    }
    
    for (const Fr& input : private_inputs) {
        full_assignment.push_back(input);
    }
    
    return full_assignment;
}

std::vector<Polynomial> QAPLegacy::matrix_to_polynomials_new(
    const R1CS& r1cs,
    const std::vector<LinearCombination>& matrix,
    const std::vector<Fr>& domain) const {
    
    std::vector<Polynomial> polynomials(num_variables);
    
    for (size_t var = 0; var < num_variables; ++var) {
        std::vector<Fr> values = r1cs.column_values(matrix, var);
        polynomials[var] = interpolate_variable(values, domain);
    }
    
    return polynomials;
}

Polynomial QAPLegacy::interpolate_variable(const std::vector<Fr>& values,
                                         const std::vector<Fr>& domain) const {
    return Polynomial::lagrange_interpolate(domain, values);
}

std::tuple<Polynomial, Polynomial, Polynomial> 
QAPLegacy::compute_abc_polynomials(const std::vector<Fr>& assignment) const {
    Polynomial A_poly, B_poly, C_poly;
    
    for (size_t i = 0; i < num_variables && i < A.size(); ++i) {
        A_poly = A_poly + (A[i] * assignment[i]);
    }
    
    for (size_t i = 0; i < num_variables && i < B.size(); ++i) {
        B_poly = B_poly + (B[i] * assignment[i]);
    }
    
    for (size_t i = 0; i < num_variables && i < C.size(); ++i) {
        C_poly = C_poly + (C[i] * assignment[i]);
    }
    
    return {A_poly, B_poly, C_poly};
}

} // namespace zkmini
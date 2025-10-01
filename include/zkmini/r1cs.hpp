#pragma once

#include "field.hpp"
#include <vector>
#include <unordered_map>
#include <initializer_list>
#include <string>

namespace zkmini {

// Type aliases for cleaner code
using VarIdx = size_t;

// A term in a linear combination: coefficient * variable
struct Term {
    VarIdx idx;    // Variable index
    Fr coeff;      // Coefficient
    
    Term(VarIdx i, const Fr& c) : idx(i), coeff(c) {}
    Term(VarIdx i, long c) : idx(i), coeff(Fr(c)) {}
};

// Linear combination: sum of terms
using LinearCombination = std::vector<Term>;

/**
 * R1CS (Rank-1 Constraint System) representation
 * 
 * Represents constraints of the form: ⟨A_k, x⟩ · ⟨B_k, x⟩ = ⟨C_k, x⟩
 * where x ∈ F_r^n is the witness vector with x_0 = 1 (constant)
 */
class R1CS {
public:
    // Core data
    size_t n_vars;     // Total number of variables (including x_0 = 1)
    size_t n_cons;     // Number of constraints
    
    // Constraint matrices A, B, C as sparse linear combinations
    std::vector<LinearCombination> A;  // A[constraint_idx] = linear combination
    std::vector<LinearCombination> B;  // B[constraint_idx] = linear combination  
    std::vector<LinearCombination> C;  // C[constraint_idx] = linear combination
    
    // Public variable indices (excluding x_0)
    std::vector<VarIdx> public_indices;
    
    // Constructors
    R1CS(size_t n_vars_hint = 1);
    
    // Variable management
    VarIdx allocate_var();
    void mark_public(VarIdx v);
    size_t num_variables() const { return n_vars; }
    size_t num_constraints() const { return n_cons; }
    const std::vector<VarIdx>& public_inputs() const { return public_indices; }
    
    // Linear combination utilities
    static LinearCombination lc_from_terms(std::initializer_list<Term> terms);
    static LinearCombination lc_from_terms(const std::vector<Term>& terms);
    static LinearCombination lc_var(VarIdx i, const Fr& c = Fr(1));
    static LinearCombination lc_const(const Fr& c);
    static void lc_add_term(LinearCombination& L, VarIdx i, const Fr& c);
    static void lc_compress(LinearCombination& L);
    
    // Constraint management
    void add_constraint(const LinearCombination& A_row,
                       const LinearCombination& B_row, 
                       const LinearCombination& C_row);
    
    // Helper constraint builders
    void add_mul(VarIdx a, VarIdx b, VarIdx c);  // a * b = c
    void add_mul_lin(const LinearCombination& A_lc, VarIdx b, const LinearCombination& C_lc);  // ⟨A,x⟩ * x_b = ⟨C,x⟩
    void add_lin_eq(const LinearCombination& L, const LinearCombination& R);  // ⟨L,x⟩ = ⟨R,x⟩
    
    // Evaluation and satisfaction
    static Fr eval_lc(const LinearCombination& L, const std::vector<Fr>& x);
    bool is_satisfied(const std::vector<Fr>& x) const;
    bool is_satisfied_verbose(const std::vector<Fr>& x, size_t& first_bad_row, 
                             Fr& L_val, Fr& R_val, Fr& O_val) const;
    
    // Column extraction for QAP conversion
    std::vector<Fr> column_values(const std::vector<LinearCombination>& M, VarIdx col) const;
    
    // Utilities
    void finalize();  // Optimize internal representation
    std::string debug_row(size_t k) const;
    
    // Legacy compatibility methods
    bool is_satisfied(const std::vector<Fr>& public_inputs,
                     const std::vector<Fr>& private_inputs) const;
    std::vector<Fr> generate_full_assignment(const std::vector<Fr>& public_inputs,
                                           const std::vector<Fr>& private_inputs) const;
    
    // Serialization (TODO: implement)
    std::vector<uint8_t> serialize() const;
    static R1CS deserialize(const std::vector<uint8_t>& data);
    static R1CS from_json(const std::string& json_str);
    std::string to_json() const;

private:
    VarIdx next_var;  // Next variable index to allocate
    
    void validate_constraint_index(size_t constraint_idx) const;
    void validate_variable_index(VarIdx var_idx) const;
    
    // Serialization helpers
    std::vector<uint8_t> serialize_matrix(const std::vector<std::vector<Term>>& matrix) const;
    static std::vector<std::vector<Term>> deserialize_matrix(const std::vector<uint8_t>& data, size_t& offset);
};

} // namespace zkmini
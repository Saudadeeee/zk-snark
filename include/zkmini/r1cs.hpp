#pragma once

#include "field.hpp"
#include <vector>
#include <unordered_map>
#include <initializer_list>
#include <string>

namespace zkmini {
using VarIdx = size_t;
struct Term {
    VarIdx idx;    
    Fr coeff;      
    
    Term(VarIdx i, const Fr& c) : idx(i), coeff(c) {}
    Term(VarIdx i, long c) : idx(i), coeff(Fr(c)) {}
};
using LinearCombination = std::vector<Term>;

class R1CS {
public:
    
    size_t n_vars;     
    size_t n_cons;     
    
    
    std::vector<LinearCombination> A;  
    std::vector<LinearCombination> B;  
    std::vector<LinearCombination> C;  
    
    
    std::vector<VarIdx> public_indices;
    
    
    R1CS(size_t n_vars_hint = 1);
    
    
    VarIdx allocate_var();
    void mark_public(VarIdx v);
    size_t num_variables() const { return n_vars; }
    size_t num_constraints() const { return n_cons; }
    const std::vector<VarIdx>& public_inputs() const { return public_indices; }
    
    
    static LinearCombination lc_from_terms(std::initializer_list<Term> terms);
    static LinearCombination lc_from_terms(const std::vector<Term>& terms);
    static LinearCombination lc_var(VarIdx i, const Fr& c = Fr(1));
    static LinearCombination lc_const(const Fr& c);
    static void lc_add_term(LinearCombination& L, VarIdx i, const Fr& c);
    static void lc_compress(LinearCombination& L);
    
    
    void add_constraint(const LinearCombination& A_row,
                       const LinearCombination& B_row, 
                       const LinearCombination& C_row);
    
    
    void add_mul(VarIdx a, VarIdx b, VarIdx c);  
    void add_mul_lin(const LinearCombination& A_lc, VarIdx b, const LinearCombination& C_lc);  
    void add_lin_eq(const LinearCombination& L, const LinearCombination& R);  
    
    
    static Fr eval_lc(const LinearCombination& L, const std::vector<Fr>& x);
    bool is_satisfied(const std::vector<Fr>& x) const;
    bool is_satisfied_verbose(const std::vector<Fr>& x, size_t& first_bad_row, 
                             Fr& L_val, Fr& R_val, Fr& O_val) const;
    
    
    std::vector<Fr> column_values(const std::vector<LinearCombination>& M, VarIdx col) const;
    
    
    void finalize();  
    std::string debug_row(size_t k) const;
    
    
    bool is_satisfied(const std::vector<Fr>& public_inputs,
                     const std::vector<Fr>& private_inputs) const;
    std::vector<Fr> generate_full_assignment(const std::vector<Fr>& public_inputs,
                                           const std::vector<Fr>& private_inputs) const;
    
    
    std::vector<uint8_t> serialize() const;
    static R1CS deserialize(const std::vector<uint8_t>& data);
    static R1CS from_json(const std::string& json_str);
    std::string to_json() const;

private:
    VarIdx next_var;  
    
    void validate_constraint_index(size_t constraint_idx) const;
    void validate_variable_index(VarIdx var_idx) const;
    
    
    std::vector<uint8_t> serialize_matrix(const std::vector<std::vector<Term>>& matrix) const;
    static std::vector<std::vector<Term>> deserialize_matrix(const std::vector<uint8_t>& data, size_t& offset);
};

}
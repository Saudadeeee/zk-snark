#include "zkmini/r1cs.hpp"
#include "zkmini/utils.hpp"
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace zkmini {

R1CS::R1CS(size_t n_vars_hint) : n_vars(1), n_cons(0), next_var(1) {
    // Reserve space for efficiency
    if (n_vars_hint > 1) {
        n_vars = n_vars_hint;
        next_var = n_vars_hint;
    }
}

// Variable management
VarIdx R1CS::allocate_var() {
    VarIdx var = next_var++;
    if (var >= n_vars) {
        n_vars = next_var;
    }
    return var;
}

void R1CS::mark_public(VarIdx v) {
    ZK_ASSERT(v > 0 && v < n_vars, "Invalid variable index for public input");
    if (std::find(public_indices.begin(), public_indices.end(), v) == public_indices.end()) {
        public_indices.push_back(v);
    }
}

// Linear combination utilities
LinearCombination R1CS::lc_from_terms(std::initializer_list<Term> terms) {
    LinearCombination lc(terms);
    lc_compress(lc);
    return lc;
}

LinearCombination R1CS::lc_from_terms(const std::vector<Term>& terms) {
    LinearCombination lc = terms;
    lc_compress(lc);
    return lc;
}

LinearCombination R1CS::lc_var(VarIdx i, const Fr& c) {
    return {Term(i, c)};
}

LinearCombination R1CS::lc_const(const Fr& c) {
    return {Term(0, c)};  // x_0 = 1, so constant c becomes c * x_0
}

void R1CS::lc_add_term(LinearCombination& L, VarIdx i, const Fr& c) {
    if (c == Fr(0)) return;  // Don't add zero terms
    L.push_back(Term(i, c));
}

void R1CS::lc_compress(LinearCombination& L) {
    if (L.empty()) return;
    
    // Use map to combine terms with same index
    std::unordered_map<VarIdx, Fr> coeff_map;
    for (const auto& term : L) {
        coeff_map[term.idx] = coeff_map[term.idx] + term.coeff;
    }
    
    // Rebuild linear combination without zero terms
    L.clear();
    for (const auto& pair : coeff_map) {
        if (pair.second != Fr(0)) {
            L.push_back(Term(pair.first, pair.second));
        }
    }
    
    // Sort by index for deterministic representation
    std::sort(L.begin(), L.end(), [](const Term& a, const Term& b) {
        return a.idx < b.idx;
    });
}

// Constraint management
void R1CS::add_constraint(const LinearCombination& A_row,
                         const LinearCombination& B_row, 
                         const LinearCombination& C_row) {
    // Make copies and compress
    LinearCombination a_compressed = A_row;
    LinearCombination b_compressed = B_row;
    LinearCombination c_compressed = C_row;
    
    lc_compress(a_compressed);
    lc_compress(b_compressed);
    lc_compress(c_compressed);
    
    // Validate all variable indices
    for (const auto& term : a_compressed) {
        ZK_ASSERT(term.idx < n_vars, "Variable index out of bounds in A row");
    }
    for (const auto& term : b_compressed) {
        ZK_ASSERT(term.idx < n_vars, "Variable index out of bounds in B row");
    }
    for (const auto& term : c_compressed) {
        ZK_ASSERT(term.idx < n_vars, "Variable index out of bounds in C row");
    }
    
    A.push_back(a_compressed);
    B.push_back(b_compressed);
    C.push_back(c_compressed);
    n_cons++;
}

// Helper constraint builders
void R1CS::add_mul(VarIdx a, VarIdx b, VarIdx c) {
    add_constraint(lc_var(a), lc_var(b), lc_var(c));
}

void R1CS::add_mul_lin(const LinearCombination& A_lc, VarIdx b, const LinearCombination& C_lc) {
    add_constraint(A_lc, lc_var(b), C_lc);
}

void R1CS::add_lin_eq(const LinearCombination& L, const LinearCombination& R) {
    // L = R  becomes  L * 1 = R
    add_constraint(L, lc_const(Fr(1)), R);
}

// Evaluation and satisfaction
Fr R1CS::eval_lc(const LinearCombination& L, const std::vector<Fr>& x) {
    Fr result = Fr(0);
    for (const auto& term : L) {
        ZK_ASSERT(term.idx < x.size(), "Variable index out of bounds in evaluation");
        result = result + (term.coeff * x[term.idx]);
    }
    return result;
}

bool R1CS::is_satisfied(const std::vector<Fr>& x) const {
    ZK_ASSERT(x.size() == n_vars, "Wrong witness size");
    ZK_ASSERT(x[0] == Fr(1), "x[0] must be 1");
    
    for (size_t k = 0; k < n_cons; ++k) {
        Fr L = eval_lc(A[k], x);
        Fr R = eval_lc(B[k], x);
        Fr O = eval_lc(C[k], x);
        
        if (!((L * R) == O)) {
            return false;
        }
    }
    
    return true;
}

bool R1CS::is_satisfied_verbose(const std::vector<Fr>& x, size_t& first_bad_row, 
                               Fr& L_val, Fr& R_val, Fr& O_val) const {
    ZK_ASSERT(x.size() == n_vars, "Wrong witness size");
    ZK_ASSERT(x[0] == Fr(1), "x[0] must be 1");
    
    for (size_t k = 0; k < n_cons; ++k) {
        Fr L = eval_lc(A[k], x);
        Fr R = eval_lc(B[k], x);
        Fr O = eval_lc(C[k], x);
        
        if (!((L * R) == O)) {
            first_bad_row = k;
            L_val = L;
            R_val = R;
            O_val = O;
            return false;
        }
    }
    
    return true;
}

// Column extraction for QAP conversion
std::vector<Fr> R1CS::column_values(const std::vector<LinearCombination>& M, VarIdx col) const {
    std::vector<Fr> vals(n_cons, Fr(0));
    
    for (size_t k = 0; k < n_cons; ++k) {
        for (const auto& term : M[k]) {
            if (term.idx == col) {
                vals[k] = term.coeff;
                break;  // Found the coefficient for this variable in this constraint
            }
        }
    }
    
    return vals;
}

// Utilities
void R1CS::finalize() {
    // Compress all linear combinations and shrink vectors
    for (auto& lc : A) {
        lc_compress(lc);
        lc.shrink_to_fit();
    }
    for (auto& lc : B) {
        lc_compress(lc);
        lc.shrink_to_fit();
    }
    for (auto& lc : C) {
        lc_compress(lc);
        lc.shrink_to_fit();
    }
    
    A.shrink_to_fit();
    B.shrink_to_fit();
    C.shrink_to_fit();
    public_indices.shrink_to_fit();
}

std::string R1CS::debug_row(size_t k) const {
    validate_constraint_index(k);
    
    std::stringstream ss;
    ss << "Constraint " << k << ":\n";
    
    // A row
    ss << "  A[" << k << "]: ";
    for (const auto& term : A[k]) {
        ss << "(" << term.idx << "," << term.coeff.to_string() << ") ";
    }
    ss << "\n";
    
    // B row  
    ss << "  B[" << k << "]: ";
    for (const auto& term : B[k]) {
        ss << "(" << term.idx << "," << term.coeff.to_string() << ") ";
    }
    ss << "\n";
    
    // C row
    ss << "  C[" << k << "]: ";
    for (const auto& term : C[k]) {
        ss << "(" << term.idx << "," << term.coeff.to_string() << ") ";
    }
    ss << "\n";
    
    return ss.str();
}

// Legacy compatibility methods
bool R1CS::is_satisfied(const std::vector<Fr>& public_inputs,
                       const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> full_assignment = generate_full_assignment(public_inputs, private_inputs);
    return is_satisfied(full_assignment);
}

std::vector<Fr> R1CS::generate_full_assignment(const std::vector<Fr>& public_inputs,
                                             const std::vector<Fr>& private_inputs) const {
    std::vector<Fr> full_assignment;
    full_assignment.reserve(n_vars);
    
    // z[0] = 1 (constant)
    full_assignment.push_back(Fr(1));
    
    // Add public inputs
    for (const Fr& input : public_inputs) {
        full_assignment.push_back(input);
    }
    
    // Add private inputs
    for (const Fr& input : private_inputs) {
        full_assignment.push_back(input);
    }
    
    // Pad with zeros if needed
    while (full_assignment.size() < n_vars) {
        full_assignment.push_back(Fr(0));
    }
    
    return full_assignment;
}

// Serialization (TODO: implement properly)
std::vector<uint8_t> R1CS::serialize() const {
    // TODO: Implement binary serialization
    return {};
}

R1CS R1CS::deserialize(const std::vector<uint8_t>& data) {
    // TODO: Implement binary deserialization
    return R1CS();
}

R1CS R1CS::from_json(const std::string& json_str) {
    // TODO: Implement JSON deserialization
    return R1CS();
}

std::string R1CS::to_json() const {
    // TODO: Implement JSON serialization
    return "{}";
}

void R1CS::validate_constraint_index(size_t constraint_idx) const {
    ZK_ASSERT(constraint_idx < n_cons, "Constraint index out of bounds");
}

void R1CS::validate_variable_index(VarIdx var_idx) const {
    ZK_ASSERT(var_idx < n_vars, "Variable index out of bounds");
}

} // namespace zkmini
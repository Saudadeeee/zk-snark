#include "zkmini/r1cs.hpp"
#include "zkmini/utils.hpp"
#include "zkmini/serialization.hpp"
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

// R1CS Serialization
std::vector<uint8_t> R1CS::serialize() const {
    std::vector<uint8_t> result;
    
    // Write dimensions
    Serialization::write_uint64(result, n_vars);
    Serialization::write_uint64(result, n_cons);
    
    // Write matrices
    auto A_data = serialize_matrix(A);
    auto B_data = serialize_matrix(B);
    auto C_data = serialize_matrix(C);
    
    Serialization::write_uint64(result, A_data.size());
    result.insert(result.end(), A_data.begin(), A_data.end());
    
    Serialization::write_uint64(result, B_data.size());
    result.insert(result.end(), B_data.begin(), B_data.end());
    
    Serialization::write_uint64(result, C_data.size());
    result.insert(result.end(), C_data.begin(), C_data.end());
    
    return result;
}

R1CS R1CS::deserialize(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    
    // Read dimensions
    uint64_t n_vars = Serialization::read_uint64(data, offset);
    uint64_t n_cons = Serialization::read_uint64(data, offset);
    
    R1CS result;
    result.n_vars = n_vars;
    result.n_cons = n_cons;
    
    // Read matrices
    uint64_t A_size = Serialization::read_uint64(data, offset);
    std::vector<uint8_t> A_data(data.begin() + offset, data.begin() + offset + A_size);
    offset += A_size;
    
    uint64_t B_size = Serialization::read_uint64(data, offset);
    std::vector<uint8_t> B_data(data.begin() + offset, data.begin() + offset + B_size);
    offset += B_size;
    
    uint64_t C_size = Serialization::read_uint64(data, offset);
    std::vector<uint8_t> C_data(data.begin() + offset, data.begin() + offset + C_size);
    
    size_t A_offset = 0, B_offset = 0, C_offset = 0;
    result.A = deserialize_matrix(A_data, A_offset);
    result.B = deserialize_matrix(B_data, B_offset);
    result.C = deserialize_matrix(C_data, C_offset);
    
    return result;
}

R1CS R1CS::from_json(const std::string& json_str) {

    R1CS result;
 
    result.n_vars = 0;
    result.n_cons = 0;
    
    return result;
}

std::string R1CS::to_json() const {
    std::string result = "{";
    result += "\"n_vars\":" + std::to_string(n_vars) + ",";
    result += "\"n_cons\":" + std::to_string(n_cons) + ",";
    result += "\"A\":[";
    
    for (size_t i = 0; i < A.size(); ++i) {
        if (i > 0) result += ",";
        result += "[";
        for (size_t j = 0; j < A[i].size(); ++j) {
            if (j > 0) result += ",";
            result += "{\"var\":" + std::to_string(A[i][j].idx) + ",\"coeff\":\"" + A[i][j].coeff.to_hex() + "\"}";
        }
        result += "]";
    }
    
    result += "],\"B\":[";
    for (size_t i = 0; i < B.size(); ++i) {
        if (i > 0) result += ",";
        result += "[";
        for (size_t j = 0; j < B[i].size(); ++j) {
            if (j > 0) result += ",";
            result += "{\"var\":" + std::to_string(B[i][j].idx) + ",\"coeff\":\"" + B[i][j].coeff.to_hex() + "\"}";
        }
        result += "]";
    }
    
    result += "],\"C\":[";
    for (size_t i = 0; i < C.size(); ++i) {
        if (i > 0) result += ",";
        result += "[";
        for (size_t j = 0; j < C[i].size(); ++j) {
            if (j > 0) result += ",";
            result += "{\"var\":" + std::to_string(C[i][j].idx) + ",\"coeff\":\"" + C[i][j].coeff.to_hex() + "\"}";
        }
        result += "]";
    }
    
    result += "]}";
    return result;
}

void R1CS::validate_constraint_index(size_t constraint_idx) const {
    ZK_ASSERT(constraint_idx < n_cons, "Constraint index out of bounds");
}

void R1CS::validate_variable_index(VarIdx var_idx) const {
    ZK_ASSERT(var_idx < n_vars, "Variable index out of bounds");
}

std::vector<uint8_t> R1CS::serialize_matrix(const std::vector<std::vector<Term>>& matrix) const {
    std::vector<uint8_t> result;
    
    Serialization::write_uint64(result, matrix.size());
    for (const auto& row : matrix) {
        Serialization::write_uint64(result, row.size());
        for (const auto& term : row) {
            Serialization::write_uint64(result, term.idx);
            auto coeff_data = Serialization::serialize_fr(term.coeff);
            result.insert(result.end(), coeff_data.begin(), coeff_data.end());
        }
    }
    
    return result;
}

std::vector<std::vector<Term>> R1CS::deserialize_matrix(const std::vector<uint8_t>& data, size_t& offset) {
    uint64_t rows = Serialization::read_uint64(data, offset);
    std::vector<std::vector<Term>> matrix;
    matrix.reserve(rows);
    
    for (size_t i = 0; i < rows; ++i) {
        uint64_t cols = Serialization::read_uint64(data, offset);
        std::vector<Term> row;
        row.reserve(cols);
        
        for (size_t j = 0; j < cols; ++j) {
            VarIdx idx = Serialization::read_uint64(data, offset);
            Fr coeff = Serialization::deserialize_fr(data, offset);
            row.emplace_back(idx, coeff);
        }
        matrix.push_back(std::move(row));
    }
    
    return matrix;
}

} // namespace zkmini
#include "zkmini/r1cs.hpp"
#include "zkmini/utils.hpp"
#include <sstream>

namespace zkmini {

R1CS::R1CS() : num_variables(0), num_public(0), num_private(0), num_constraints(0) {}

R1CS::R1CS(size_t num_variables, size_t num_public, size_t num_constraints) 
    : num_variables(num_variables), num_public(num_public), 
      num_private(num_variables - num_public - 1), num_constraints(num_constraints) {
    A.resize(num_constraints);
    B.resize(num_constraints);
    C.resize(num_constraints);
}

void R1CS::add_constraint(const std::vector<std::pair<size_t, Fr>>& a_terms,
                         const std::vector<std::pair<size_t, Fr>>& b_terms,
                         const std::vector<std::pair<size_t, Fr>>& c_terms) {
    A.push_back(a_terms);
    B.push_back(b_terms);
    C.push_back(c_terms);
    num_constraints++;
}

bool R1CS::is_satisfied(const std::vector<Fr>& public_inputs,
                       const std::vector<Fr>& private_inputs) const {
    ZK_ASSERT(public_inputs.size() == num_public, "Wrong number of public inputs");
    ZK_ASSERT(private_inputs.size() == num_private, "Wrong number of private inputs");
    
    std::vector<Fr> full_assignment = generate_full_assignment(public_inputs, private_inputs);
    
    for (size_t i = 0; i < num_constraints; ++i) {
        Fr a_val, b_val, c_val;
        
        // Compute A·z
        for (const auto& term : A[i]) {
            a_val = a_val + (term.second * full_assignment[term.first]);
        }
        
        // Compute B·z
        for (const auto& term : B[i]) {
            b_val = b_val + (term.second * full_assignment[term.first]);
        }
        
        // Compute C·z
        for (const auto& term : C[i]) {
            c_val = c_val + (term.second * full_assignment[term.first]);
        }
        
        // Check A·z * B·z = C·z
        if (!((a_val * b_val) == c_val)) {
            return false;
        }
    }
    
    return true;
}

std::vector<Fr> R1CS::generate_full_assignment(const std::vector<Fr>& public_inputs,
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

std::vector<Fr> R1CS::matrix_vector_mul(
    const std::vector<std::vector<std::pair<size_t, Fr>>>& matrix,
    const std::vector<Fr>& vector) const {
    
    std::vector<Fr> result(matrix.size(), Fr());
    
    for (size_t i = 0; i < matrix.size(); ++i) {
        for (const auto& term : matrix[i]) {
            result[i] = result[i] + (term.second * vector[term.first]);
        }
    }
    
    return result;
}

R1CS::Constraint R1CS::get_constraint(size_t i) const {
    validate_constraint_index(i);
    return {A[i], B[i], C[i]};
}

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
    ZK_ASSERT(constraint_idx < num_constraints, "Constraint index out of bounds");
}

void R1CS::validate_variable_index(size_t var_idx) const {
    ZK_ASSERT(var_idx < num_variables, "Variable index out of bounds");
}

} // namespace zkmini
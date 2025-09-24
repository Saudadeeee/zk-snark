#pragma once

#include "field.hpp"
#include <vector>
#include <unordered_map>

namespace zkmini {

// Placeholder for R1CS (Rank-1 Constraint System) representation
// TODO: Implement R1CS constraint system
class R1CS {
public:
    size_t num_variables;    // Total number of variables (including 1, public, private)
    size_t num_public;       // Number of public inputs (excluding constant 1)
    size_t num_private;      // Number of private inputs (witness)
    size_t num_constraints;  // Number of constraints
    
    // Constraint matrices A, B, C where A·z ⊙ B·z = C·z
    // z = [1, public_inputs, private_inputs]
    std::vector<std::vector<std::pair<size_t, Fr>>> A; // A[constraint][variable] = coefficient
    std::vector<std::vector<std::pair<size_t, Fr>>> B; // Sparse representation
    std::vector<std::vector<std::pair<size_t, Fr>>> C;
    
    R1CS();
    R1CS(size_t num_variables, size_t num_public, size_t num_constraints);
    
    // Add a constraint: (Σ a_i * z_i) * (Σ b_i * z_i) = (Σ c_i * z_i)
    void add_constraint(const std::vector<std::pair<size_t, Fr>>& a_terms,
                       const std::vector<std::pair<size_t, Fr>>& b_terms,
                       const std::vector<std::pair<size_t, Fr>>& c_terms);
    
    // Check if a witness satisfies the R1CS
    bool is_satisfied(const std::vector<Fr>& public_inputs,
                     const std::vector<Fr>& private_inputs) const;
    
    // Generate full assignment vector z = [1, public, private]
    std::vector<Fr> generate_full_assignment(const std::vector<Fr>& public_inputs,
                                           const std::vector<Fr>& private_inputs) const;
    
    // Matrix-vector multiplication: M * z
    std::vector<Fr> matrix_vector_mul(const std::vector<std::vector<std::pair<size_t, Fr>>>& matrix,
                                     const std::vector<Fr>& vector) const;
    
    // Get constraint at index i
    struct Constraint {
        std::vector<std::pair<size_t, Fr>> a, b, c;
    };
    Constraint get_constraint(size_t i) const;
    
    // Serialize/deserialize
    std::vector<uint8_t> serialize() const;
    static R1CS deserialize(const std::vector<uint8_t>& data);
    
    // Load from JSON format
    static R1CS from_json(const std::string& json_str);
    std::string to_json() const;

private:
    void validate_constraint_index(size_t constraint_idx) const;
    void validate_variable_index(size_t var_idx) const;
};

} // namespace zkmini
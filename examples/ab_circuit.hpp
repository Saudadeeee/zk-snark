#pragma once

#include "zkmini/r1cs.hpp"
#include "zkmini/field.hpp"

namespace zkmini {
namespace examples {

// Simple circuit: a * b = c
// Variables: [1, a, b, c] (indices 0, 1, 2, 3)
// Public inputs: c (index 3)
// Private inputs: a, b (indices 1, 2)
// Constraint: a * b = c
class ABCircuit {
public:
    static R1CS create_r1cs() {
        // 4 variables: [1, a, b, c]
        // 1 public input: c
        // 1 constraint: a * b = c
        R1CS r1cs(4, 1, 1);
        
        // Constraint: a * b = c
        // A terms: coefficient 1 for variable 'a' (index 1)
        std::vector<std::pair<size_t, Fr>> a_terms = {{1, Fr(1)}};
        
        // B terms: coefficient 1 for variable 'b' (index 2)
        std::vector<std::pair<size_t, Fr>> b_terms = {{2, Fr(1)}};
        
        // C terms: coefficient 1 for variable 'c' (index 3)
        std::vector<std::pair<size_t, Fr>> c_terms = {{3, Fr(1)}};
        
        r1cs.add_constraint(a_terms, b_terms, c_terms);
        
        return r1cs;
    }
    
    // Create witness for a * b = c
    static std::pair<std::vector<Fr>, std::vector<Fr>> create_witness(const Fr& a, const Fr& b) {
        Fr c = a * b;
        
        // Public inputs: [c]
        std::vector<Fr> public_inputs = {c};
        
        // Private inputs: [a, b]
        std::vector<Fr> private_inputs = {a, b};
        
        return {public_inputs, private_inputs};
    }
    
    // Verify that witness satisfies the circuit
    static bool verify_witness(const Fr& a, const Fr& b, const Fr& c) {
        return (a * b) == c;
    }
    
    // Create a sample instance
    static std::tuple<R1CS, std::vector<Fr>, std::vector<Fr>> create_sample() {
        R1CS r1cs = create_r1cs();
        
        // Sample values: a = 3, b = 4, c = 12
        Fr a(3);
        Fr b(4);
        
        auto [public_inputs, private_inputs] = create_witness(a, b);
        
        return {r1cs, public_inputs, private_inputs};
    }
};

// More complex circuit: quadratic equation
// Prove knowledge of x such that x^2 + x + 5 = y
// Variables: [1, x, x_squared, y] (indices 0, 1, 2, 3)
// Constraints:
//   1. x * x = x_squared
//   2. x_squared + x + 5 = y
class QuadraticCircuit {
public:
    static R1CS create_r1cs() {
        // 4 variables: [1, x, x_squared, y]
        // 1 public input: y
        // 2 constraints
        R1CS r1cs(4, 1, 2);
        
        // Constraint 1: x * x = x_squared
        std::vector<std::pair<size_t, Fr>> a1_terms = {{1, Fr(1)}}; // x
        std::vector<std::pair<size_t, Fr>> b1_terms = {{1, Fr(1)}}; // x
        std::vector<std::pair<size_t, Fr>> c1_terms = {{2, Fr(1)}}; // x_squared
        r1cs.add_constraint(a1_terms, b1_terms, c1_terms);
        
        // Constraint 2: 1 * (x_squared + x + 5) = y
        // Rewritten as: 1 * y = x_squared + x + 5
        std::vector<std::pair<size_t, Fr>> a2_terms = {{0, Fr(1)}}; // 1
        std::vector<std::pair<size_t, Fr>> b2_terms = {{3, Fr(1)}}; // y
        std::vector<std::pair<size_t, Fr>> c2_terms = {
            {0, Fr(5)},  // 5 * 1
            {1, Fr(1)},  // 1 * x
            {2, Fr(1)}   // 1 * x_squared
        };
        r1cs.add_constraint(a2_terms, b2_terms, c2_terms);
        
        return r1cs;
    }
    
    // Create witness for x^2 + x + 5 = y
    static std::pair<std::vector<Fr>, std::vector<Fr>> create_witness(const Fr& x) {
        Fr x_squared = x * x;
        Fr y = x_squared + x + Fr(5);
        
        // Public inputs: [y]
        std::vector<Fr> public_inputs = {y};
        
        // Private inputs: [x, x_squared]
        std::vector<Fr> private_inputs = {x, x_squared};
        
        return {public_inputs, private_inputs};
    }
    
    // Create sample instance
    static std::tuple<R1CS, std::vector<Fr>, std::vector<Fr>> create_sample() {
        R1CS r1cs = create_r1cs();
        
        // Sample: x = 7, so y = 49 + 7 + 5 = 61
        Fr x(7);
        
        auto [public_inputs, private_inputs] = create_witness(x);
        
        return {r1cs, public_inputs, private_inputs};
    }
};

} // namespace examples
} // namespace zkmini
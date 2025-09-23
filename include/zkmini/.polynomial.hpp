#pragma once

#include "field.hpp"
#include <vector>

namespace zkmini {

// Placeholder for polynomial operations over Fr
// TODO: Implement polynomial arithmetic and evaluation
class Polynomial {
public:
    std::vector<Fr> coeffs; // coeffs[i] is coefficient of x^i
    
    Polynomial();
    Polynomial(const std::vector<Fr>& coeffs);
    Polynomial(size_t degree); // Zero polynomial of given degree
    
    // Basic operations
    Polynomial operator+(const Polynomial& other) const;
    Polynomial operator-(const Polynomial& other) const;
    Polynomial operator*(const Polynomial& other) const;
    Polynomial operator*(const Fr& scalar) const;
    
    bool operator==(const Polynomial& other) const;
    
    // Evaluation
    Fr evaluate(const Fr& x) const;
    std::vector<Fr> evaluate_batch(const std::vector<Fr>& x_vec) const;
    
    // Degree and properties
    size_t degree() const;
    bool is_zero() const;
    Fr leading_coefficient() const;
    
    // Polynomial division
    std::pair<Polynomial, Polynomial> divide(const Polynomial& divisor) const;
    
    // Lagrange interpolation
    static Polynomial lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                         const std::vector<Fr>& y_coords);
    
    // Vanishing polynomial: (x - r1)(x - r2)...(x - rn)
    static Polynomial vanishing_polynomial(const std::vector<Fr>& roots);
    
    // Random polynomial of given degree
    static Polynomial random(size_t degree);
    
    // Utilities
    void resize(size_t new_size);
    void trim(); // Remove leading zeros
    
private:
    void ensure_size(size_t size);
};

} // namespace zkmini
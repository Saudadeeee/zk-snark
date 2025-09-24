#pragma once

#include "field.hpp"
#include <vector>
#include <string>

namespace zkmini {

// Polynomial operations over Fr with dense representation
// Invariant: polynomials are always normalized (trailing zeros removed)
// Convention: deg(0) = -1
class Polynomial {
public:
    std::vector<Fr> coeffs; // coeffs[i] is coefficient of x^i
    
    // Constructors
    Polynomial();
    Polynomial(const std::vector<Fr>& coeffs);
    Polynomial(size_t degree); // Zero polynomial of given degree
    
    // Factory methods
    static Polynomial zero();
    static Polynomial one();
    static Polynomial from_coeffs(const std::vector<Fr>& coeffs);
    
    // Access/update methods
    int deg() const; // Returns -1 for zero polynomial
    size_t size() const;
    const Fr& coeff(size_t i) const; // Returns 0 if i >= size
    void set_coeff(size_t i, const Fr& v); // Auto-grow, assign, normalize if needed
    
    // Normalization
    void normalize();
    
    // Basic arithmetic operations
    static Polynomial add(const Polynomial& a, const Polynomial& b);
    static Polynomial sub(const Polynomial& a, const Polynomial& b);
    static void add_inplace(Polynomial& dst, const Polynomial& src);
    static void sub_inplace(Polynomial& dst, const Polynomial& src);
    
    // Scalar multiplication
    static Polynomial scalar_mul(const Polynomial& f, const Fr& k);
    static void scalar_mul_inplace(Polynomial& f, const Fr& k);
    
    // Polynomial multiplication
    static Polynomial mul_schoolbook(const Polynomial& f, const Polynomial& g);
    static Polynomial square(const Polynomial& f);
    
    // Evaluation using Horner's method
    static Fr eval(const Polynomial& f, const Fr& x);
    
    // Shift operations (multiply by x^k)
    static Polynomial mul_xk(const Polynomial& f, size_t k);
    
    // Polynomial division
    static void divrem(const Polynomial& N, const Polynomial& D, Polynomial& Q, Polynomial& R);
    
    // Vanishing polynomial construction
    static Polynomial vanishing(const std::vector<Fr>& points);
    
    // Lagrange interpolation
    static Polynomial lagrange_basis(const std::vector<Fr>& pts, size_t j);
    static Polynomial interpolate(const std::vector<Fr>& pts, const std::vector<Fr>& vals);
    
    // Utility methods
    bool is_zero() const;
    std::string to_string() const;
    bool equals(const Polynomial& other) const;
    
    // Operator overloads (convenience wrappers)
    Polynomial operator+(const Polynomial& other) const;
    Polynomial operator-(const Polynomial& other) const;
    Polynomial operator*(const Polynomial& other) const;
    Polynomial operator*(const Fr& scalar) const;
    bool operator==(const Polynomial& other) const;
    
    // Evaluation
    Fr evaluate(const Fr& x) const;
    std::vector<Fr> evaluate_batch(const std::vector<Fr>& x_vec) const;
    
    // Degree and properties (legacy compatibility)
    size_t degree() const;
    Fr leading_coefficient() const;
    
    // Polynomial division (legacy compatibility)
    std::pair<Polynomial, Polynomial> divide(const Polynomial& divisor) const;
    
    // Lagrange interpolation (legacy compatibility)
    static Polynomial lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                         const std::vector<Fr>& y_coords);
    
    // Vanishing polynomial (legacy compatibility)
    static Polynomial vanishing_polynomial(const std::vector<Fr>& roots);
    
    // Random polynomial of given degree
    static Polynomial random(size_t degree);
    
    // Utilities (legacy compatibility)
    void resize(size_t new_size);
    void trim(); // Remove leading zeros
    
private:
    void ensure_size(size_t size);
    static const Fr ZERO_FR; // Static zero for returning references
};

} // namespace zkmini
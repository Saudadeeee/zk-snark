#pragma once

#include "field.hpp"
#include <vector>
#include <string>

namespace zkmini {
class Polynomial {
public:
    std::vector<Fr> coeffs; 
    
    
    Polynomial();
    Polynomial(const std::vector<Fr>& coeffs);
    Polynomial(size_t degree); 
    
    
    static Polynomial zero();
    static Polynomial one();
    static Polynomial from_coeffs(const std::vector<Fr>& coeffs);
    
    
    int deg() const; 
    size_t size() const;
    const Fr& coeff(size_t i) const; 
    void set_coeff(size_t i, const Fr& v); 
    const std::vector<Fr>& coefficients() const { return coeffs; } 
    
    
    void normalize();
    
    
    static Polynomial add(const Polynomial& a, const Polynomial& b);
    static Polynomial sub(const Polynomial& a, const Polynomial& b);
    static void add_inplace(Polynomial& dst, const Polynomial& src);
    static void sub_inplace(Polynomial& dst, const Polynomial& src);
    
    
    static Polynomial scalar_mul(const Polynomial& f, const Fr& k);
    static void scalar_mul_inplace(Polynomial& f, const Fr& k);
    
    
    static Polynomial mul_schoolbook(const Polynomial& f, const Polynomial& g);
    static Polynomial square(const Polynomial& f);
    
    
    static Fr eval(const Polynomial& f, const Fr& x);
    
    
    static Polynomial mul_xk(const Polynomial& f, size_t k);
    
    
    static void divrem(const Polynomial& N, const Polynomial& D, Polynomial& Q, Polynomial& R);
    
    
    static Polynomial vanishing(const std::vector<Fr>& points);
    
    
    static Polynomial lagrange_basis(const std::vector<Fr>& pts, size_t j);
    static Polynomial interpolate(const std::vector<Fr>& pts, const std::vector<Fr>& vals);
    
    
    bool is_zero() const;
    std::string to_string() const;
    bool equals(const Polynomial& other) const;
    
    
    Polynomial operator+(const Polynomial& other) const;
    Polynomial operator-(const Polynomial& other) const;
    Polynomial operator*(const Polynomial& other) const;
    Polynomial operator*(const Fr& scalar) const;
    bool operator==(const Polynomial& other) const;
    
    
    Fr evaluate(const Fr& x) const;
    std::vector<Fr> evaluate_batch(const std::vector<Fr>& x_vec) const;
    
    
    size_t degree() const;
    Fr leading_coefficient() const;
    
    
    std::pair<Polynomial, Polynomial> divide(const Polynomial& divisor) const;
    
    
    static Polynomial lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                         const std::vector<Fr>& y_coords);
    
    
    static Polynomial vanishing_polynomial(const std::vector<Fr>& roots);
    
    
    static Polynomial random(size_t degree);
    
    
    void resize(size_t new_size);
    void trim(); 
    
private:
    void ensure_size(size_t size);
    static const Fr ZERO_FR; 
};

}
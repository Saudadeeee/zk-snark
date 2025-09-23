#pragma once

#include "field.hpp"
#include ".fq.hpp"

namespace zkmini {

// Placeholder for G1 elliptic curve group (over Fq)
// TODO: Implement G1 point operations in Jacobian coordinates
class G1 {
public:
    Fq x, y, z; // Jacobian coordinates (x/z^2, y/z^3)
    
    G1(); // Point at infinity
    G1(const Fq& x, const Fq& y);
    G1(const Fq& x, const Fq& y, const Fq& z);
    
    // Point operations
    G1 operator+(const G1& other) const;
    G1 operator-(const G1& other) const;
    G1 operator*(const Fr& scalar) const; // Scalar multiplication
    
    bool operator==(const G1& other) const;
    bool is_zero() const; // Point at infinity
    bool is_on_curve() const;
    
    G1 double_point() const;
    G1 negate() const;
    
    // Convert to affine coordinates
    std::pair<Fq, Fq> to_affine() const;
    
    // Generator point
    static G1 generator();
    
    // Random point
    static G1 random();
};

} // namespace zkmini
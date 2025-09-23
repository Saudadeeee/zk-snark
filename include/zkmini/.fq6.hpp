#pragma once

#include ".fq2.hpp"

namespace zkmini {

// Placeholder for Fq6 = Fq2[v]/(v^3 - (9 + u)) extension field
// TODO: Implement Fq6 cubic extension field
class Fq6 {
public:
    Fq2 c0, c1, c2; // c0 + c1*v + c2*v^2
    
    Fq6();
    Fq6(const Fq2& c0, const Fq2& c1, const Fq2& c2);
    
    Fq6 operator+(const Fq6& other) const;
    Fq6 operator-(const Fq6& other) const;
    Fq6 operator*(const Fq6& other) const;
    
    bool operator==(const Fq6& other) const;
    bool is_zero() const;
    
    Fq6 inverse() const;
    Fq6 square() const;
    
    // Frobenius map
    Fq6 frobenius_map(uint64_t power) const;
    
    // Multiplication by sparse elements
    Fq6 mul_by_034(const Fq2& c0, const Fq2& c3, const Fq2& c4) const;
};

} // namespace zkmini
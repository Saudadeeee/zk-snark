#pragma once

#include ".fq.hpp"

namespace zkmini {

// Placeholder for Fq2 = Fq[u]/(u^2 + 1) extension field
// TODO: Implement Fq2 quadratic extension field
class Fq2 {
public:
    Fq c0, c1; // c0 + c1*u
    
    Fq2();
    Fq2(const Fq& c0, const Fq& c1);
    
    Fq2 operator+(const Fq2& other) const;
    Fq2 operator-(const Fq2& other) const;
    Fq2 operator*(const Fq2& other) const;
    
    bool operator==(const Fq2& other) const;
    bool is_zero() const;
    
    Fq2 inverse() const;
    Fq2 square() const;
    Fq2 conjugate() const;
    
    // Frobenius map
    Fq2 frobenius_map(uint64_t power) const;
};

} // namespace zkmini
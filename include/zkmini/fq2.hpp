#pragma once

#include "fq.hpp"

namespace zkmini {

class Fq2 {
public:
    static const Fq NON_RESIDUE;
    
    Fq c0, c1;
    
    Fq2();
    Fq2(const Fq& c0, const Fq& c1);
    
    Fq2 operator+(const Fq2& other) const;
    Fq2 operator-(const Fq2& other) const;
    Fq2 operator*(const Fq2& other) const;
    
    bool operator==(const Fq2& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    Fq2 inverse() const;
    Fq2 square() const;
    Fq2 conjugate() const;
    
    Fq2 frobenius_map(uint64_t power) const;
    Fq2 mul_by_nonresidue() const;
};

} // namespace zkmini
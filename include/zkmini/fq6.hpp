#pragma once

#include "fq2.hpp"

namespace zkmini {

class Fq6 {
public:
    static const Fq2 NON_RESIDUE;
    
    Fq2 c0, c1, c2;
    
    Fq6();
    Fq6(const Fq2& c0, const Fq2& c1, const Fq2& c2);
    
    Fq6 operator+(const Fq6& other) const;
    Fq6 operator-(const Fq6& other) const;
    Fq6 operator*(const Fq6& other) const;
    
    bool operator==(const Fq6& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    Fq6 inverse() const;
    Fq6 square() const;
    
    Fq6 frobenius_map(uint64_t power) const;
    Fq6 mul_by_034(const Fq2& c0, const Fq2& c3, const Fq2& c4) const;
    Fq6 mul_by_nonresidue() const;
};

}
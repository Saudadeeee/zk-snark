#pragma once

#include "field.hpp"
#include "fq2.hpp"

namespace zkmini {

class G2 {
public:
    Fq2 x, y, z;
    
    G2(); 
    G2(const Fq2& x, const Fq2& y);
    G2(const Fq2& x, const Fq2& y, const Fq2& z);
    
    G2 operator+(const G2& other) const;
    G2 operator-(const G2& other) const;
    G2 operator*(const Fr& scalar) const;
    
    bool operator==(const G2& other) const;
    bool is_zero() const;
    bool is_on_curve() const;
    
    G2 double_point() const;
    G2 negate() const;
    
    std::pair<Fq2, Fq2> to_affine() const;
    
    static G2 generator();
    
    static G2 random();
    
    G2 frobenius_map(uint64_t power) const;
};

}
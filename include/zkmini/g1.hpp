#pragma once

#include "field.hpp"
#include "fq.hpp"

namespace zkmini {

class G1 {
public:
    Fq x, y, z;
    
    G1(); 
    G1(const Fq& x, const Fq& y);
    G1(const Fq& x, const Fq& y, const Fq& z);
    
    G1 operator+(const G1& other) const;
    G1 operator-(const G1& other) const;
    G1 operator*(const Fr& scalar) const;
    
    bool operator==(const G1& other) const;
    bool is_zero() const;
    bool is_on_curve() const;
    
    G1 double_point() const;
    G1 negate() const;
    
    std::pair<Fq, Fq> to_affine() const;
    
    static G1 generator();
    
    static G1 random();
};

}
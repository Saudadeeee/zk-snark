#pragma once

#include "fq6.hpp"

namespace zkmini {

class Fq12 {
public:
    Fq6 c0, c1;
    
    Fq12();
    Fq12(const Fq6& c0, const Fq6& c1);
    
    Fq12 operator+(const Fq12& other) const;
    Fq12 operator-(const Fq12& other) const;
    Fq12 operator*(const Fq12& other) const;
    
    bool operator==(const Fq12& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    Fq12 inverse() const;
    Fq12 square() const;
    Fq12 conjugate() const;
    
    Fq12 frobenius_map(uint64_t power) const;
    Fq12 final_exponentiation() const;
    Fq12 cyclotomic_square() const;
    Fq12 cyclotomic_exp(const std::vector<bool>& exp) const;
};

}
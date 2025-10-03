#include "zkmini/fq6.hpp"

namespace zkmini {

const Fq2 Fq6::NON_RESIDUE = Fq2(Fq(9), Fq(1));

Fq6::Fq6() : c0(), c1(), c2() {}

Fq6::Fq6(const Fq2& c0, const Fq2& c1, const Fq2& c2) : c0(c0), c1(c1), c2(c2) {}

Fq6 Fq6::operator+(const Fq6& other) const {
    return Fq6(c0 + other.c0, c1 + other.c1, c2 + other.c2);
}

Fq6 Fq6::operator-(const Fq6& other) const {
    return Fq6(c0 - other.c0, c1 - other.c1, c2 - other.c2);
}

Fq6 Fq6::operator*(const Fq6& other) const {
    Fq2 a_a = c0 * other.c0;
    Fq2 b_b = c1 * other.c1;
    Fq2 c_c = c2 * other.c2;
    
    Fq2 t1 = (c1 + c2) * (other.c1 + other.c2) - b_b - c_c;
    t1 = t1 * NON_RESIDUE + a_a;
    
    Fq2 t2 = (c0 + c1) * (other.c0 + other.c1) - a_a - b_b;
    t2 = t2 + c_c * NON_RESIDUE;
    
    Fq2 t3 = (c0 + c2) * (other.c0 + other.c2) - a_a - c_c + b_b;
    
    return Fq6(t1, t2, t3);
}

bool Fq6::operator==(const Fq6& other) const {
    return c0 == other.c0 && c1 == other.c1 && c2 == other.c2;
}

bool Fq6::is_zero() const {
    return c0.is_zero() && c1.is_zero() && c2.is_zero();
}

bool Fq6::is_one() const {
    return c0.is_one() && c1.is_zero() && c2.is_zero();
}

Fq6 Fq6::inverse() const {
    if (is_zero()) return Fq6();
    
    Fq2 c0_2 = c0.square();
    Fq2 c1_2 = c1.square();
    Fq2 c2_2 = c2.square();
    
    Fq2 c0_c1 = c0 * c1;
    Fq2 c0_c2 = c0 * c2;
    Fq2 c1_c2 = c1 * c2;
    
    Fq2 s0 = c0_2 - c1_c2 * NON_RESIDUE;
    Fq2 s1 = c2_2 * NON_RESIDUE - c0_c1;
    Fq2 s2 = c1_2 - c0_c2;
    
    Fq2 a1 = c2 * s1;
    Fq2 a2 = c1 * s2;
    Fq2 a3 = (a1 + a2) * NON_RESIDUE;
    
    Fq2 t = (c0 * s0 + a3).inverse();
    
    return Fq6(t * s0, t * s1, t * s2);
}

Fq6 Fq6::square() const {
    Fq2 s0 = c0.square();
    Fq2 ab = c0 * c1;
    Fq2 s1 = ab + ab;
    Fq2 s2 = (c0 - c1 + c2).square();
    Fq2 bc = c1 * c2;
    Fq2 s3 = bc + bc;
    Fq2 s4 = c2.square();
    
    Fq2 c0_new = s0 + s3 * NON_RESIDUE;
    Fq2 c1_new = s1 + s4 * NON_RESIDUE;
    Fq2 c2_new = s1 + s2 + s3 - s0 - s4;
    
    return Fq6(c0_new, c1_new, c2_new);
}

Fq6 Fq6::frobenius_map(uint64_t power) const {
    static const Fq2 FROBENIUS_COEFF_FQ6_C1[] = {
        Fq2(Fq(1), Fq(0)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000ULL), Fq(0)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd46ULL), Fq(0))
    };
    
    static const Fq2 FROBENIUS_COEFF_FQ6_C2[] = {
        Fq2(Fq(1), Fq(0)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd46ULL), Fq(0)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000ULL), Fq(0))
    };
    
    Fq2 new_c0 = c0.frobenius_map(power);
    Fq2 new_c1 = c1.frobenius_map(power) * FROBENIUS_COEFF_FQ6_C1[power % 6];
    Fq2 new_c2 = c2.frobenius_map(power) * FROBENIUS_COEFF_FQ6_C2[power % 6];
    
    return Fq6(new_c0, new_c1, new_c2);
}

Fq6 Fq6::mul_by_034(const Fq2& c0, const Fq2& c3, const Fq2& c4) const {
    Fq2 a_a = this->c0 * c0;
    Fq2 b_b = this->c1 * c3;
    Fq2 c_c = this->c2 * c4;
    
    Fq2 t1 = (this->c1 + this->c2) * (c3 + c4) - b_b - c_c;
    t1 = t1 * NON_RESIDUE + a_a;
    
    Fq2 t2 = (this->c0 + this->c1) * (c0 + c3) - a_a - b_b;
    t2 = t2 + c_c * NON_RESIDUE;
    
    Fq2 t3 = (this->c0 + this->c2) * (c0 + c4) - a_a - c_c + b_b;
    
    return Fq6(t1, t2, t3);
}

Fq6 Fq6::mul_by_nonresidue() const {
    return Fq6(c2 * NON_RESIDUE, c0, c1);
}

}
#include "zkmini/fq2.hpp"

namespace zkmini {

const Fq Fq2::NON_RESIDUE = Fq(0x3c208c16d87cfd46ULL);

Fq2::Fq2() : c0(), c1() {}

Fq2::Fq2(const Fq& c0, const Fq& c1) : c0(c0), c1(c1) {}

Fq2 Fq2::operator+(const Fq2& other) const {
    return Fq2(c0 + other.c0, c1 + other.c1);
}

Fq2 Fq2::operator-(const Fq2& other) const {
    return Fq2(c0 - other.c0, c1 - other.c1);
}

Fq2 Fq2::operator*(const Fq2& other) const {
    Fq aa = c0 * other.c0;
    Fq bb = c1 * other.c1;
    Fq o = (c0 + c1) * (other.c0 + other.c1);
    return Fq2(aa + bb * NON_RESIDUE, o - aa - bb);
}

bool Fq2::operator==(const Fq2& other) const {
    return c0 == other.c0 && c1 == other.c1;
}

bool Fq2::is_zero() const {
    return c0.is_zero() && c1.is_zero();
}

bool Fq2::is_one() const {
    return c0.is_one() && c1.is_zero();
}

Fq2 Fq2::inverse() const {
    if (is_zero()) return Fq2();
    
    Fq norm = c0.square() - c1.square() * NON_RESIDUE;
    Fq norm_inv = norm.inverse();
    
    return Fq2(c0 * norm_inv, (Fq() - c1) * norm_inv);
}

Fq2 Fq2::square() const {
    Fq a = c0 * c1;
    Fq c0_plus_c1 = c0 + c1;
    Fq c0_minus_beta_c1 = c0 + c1 * NON_RESIDUE;
    
    return Fq2(c0_plus_c1 * c0_minus_beta_c1 - a - a * NON_RESIDUE, a + a);
}

Fq2 Fq2::conjugate() const {
    return Fq2(c0, Fq() - c1);
}

Fq2 Fq2::frobenius_map(uint64_t power) const {
    if (power % 2 == 0) {
        return *this;
    }
    return Fq2(c0, Fq() - c1);
}

Fq2 Fq2::mul_by_nonresidue() const {
    return Fq2(c0 * NON_RESIDUE + c1 * NON_RESIDUE, c0 + c1);
}

}
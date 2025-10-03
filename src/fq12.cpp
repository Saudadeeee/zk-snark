#include "zkmini/fq12.hpp"

namespace zkmini {

Fq12::Fq12() : c0(), c1() {}

Fq12::Fq12(const Fq6& c0, const Fq6& c1) : c0(c0), c1(c1) {}

Fq12 Fq12::operator+(const Fq12& other) const {
    return Fq12(c0 + other.c0, c1 + other.c1);
}

Fq12 Fq12::operator-(const Fq12& other) const {
    return Fq12(c0 - other.c0, c1 - other.c1);
}

Fq12 Fq12::operator*(const Fq12& other) const {
    Fq6 aa = c0 * other.c0;
    Fq6 bb = c1 * other.c1;
    Fq6 o = (c0 + c1) * (other.c0 + other.c1);
    
    return Fq12(aa + bb.mul_by_nonresidue(), o - aa - bb);
}

bool Fq12::operator==(const Fq12& other) const {
    return c0 == other.c0 && c1 == other.c1;
}

bool Fq12::is_zero() const {
    return c0.is_zero() && c1.is_zero();
}

bool Fq12::is_one() const {
    return c0.is_one() && c1.is_zero();
}

Fq12 Fq12::inverse() const {
    if (is_zero()) return Fq12();
    
    Fq6 t0 = c0.square();
    Fq6 t1 = c1.square();
    t1 = t1.mul_by_nonresidue();
    t0 = t0 - t1;
    
    Fq6 t = t0.inverse();
    
    return Fq12(c0 * t, (Fq6() - c1) * t);
}

Fq12 Fq12::square() const {
    Fq6 ab = c0 * c1;
    Fq6 c0_plus_c1 = c0 + c1;
    Fq6 c0_plus_beta_c1 = c0 + c1.mul_by_nonresidue();
    
    return Fq12(c0_plus_c1 * c0_plus_beta_c1 - ab - ab.mul_by_nonresidue(), ab + ab);
}

Fq12 Fq12::conjugate() const {
    return Fq12(c0, Fq6() - c1);
}

Fq12 Fq12::frobenius_map(uint64_t power) const {
    static const Fq2 FROBENIUS_COEFF_FQ12_C1[] = {
        Fq2(Fq(1), Fq(0)),
        Fq2(Fq(0x1904d3bf02bb0667c231beb4202c0d1f0fd603fd3cbd5f4f7b2443d784bab9c4ULL),
            Fq(0x5f19672fdf76ce51ba69c6076a0f77eaddb3a93be6f89688de17d813620a00022ULL)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d97816a916871ca8d3c208c16d87cfd46ULL), Fq(0)),
        Fq2(Fq(0x5f19672fdf76ce51ba69c6076a0f77eaddb3a93be6f89688de17d813620a00022ULL),
            Fq(0x1904d3bf02bb0667c231beb4202c0d1f0fd603fd3cbd5f4f7b2443d784bab9c4ULL)),
        Fq2(Fq(0x30644e72e131a029b85045b68181585d2833e84879b9709143e1f593f0000000ULL), Fq(0)),
        Fq2(Fq(0x5f19672fdf76ce51ba69c6076a0f77eaddb3a93be6f89688de17d813620a00022ULL),
            Fq(0x1904d3bf02bb0667c231beb4202c0d1f0fd603fd3cbd5f4f7b2443d784bab9c4ULL))
    };
    
    Fq6 new_c0 = c0.frobenius_map(power);
    Fq6 new_c1 = c1.frobenius_map(power);
    
    new_c1 = Fq6(new_c1.c0 * FROBENIUS_COEFF_FQ12_C1[power % 12],
                 new_c1.c1 * FROBENIUS_COEFF_FQ12_C1[power % 12],
                 new_c1.c2 * FROBENIUS_COEFF_FQ12_C1[power % 12]);
    
    return Fq12(new_c0, new_c1);
}

Fq12 Fq12::final_exponentiation() const {
    Fq12 f1 = conjugate();
    Fq12 f2 = inverse();
    Fq12 r = f1 * f2;
    
    Fq12 f3 = r.frobenius_map(2);
    r = f3 * r;
    
    Fq12 y0 = r.square();
    y0 = y0.square();
    y0 = y0.square();
    
    Fq12 y1 = r.cyclotomic_square();
    Fq12 y2 = y1.cyclotomic_square();
    Fq12 y3 = y2 * y1;
    Fq12 y4 = y3.cyclotomic_square();
    Fq12 y5 = y4 * y1;
    
    return y0 * y5;
}

Fq12 Fq12::cyclotomic_square() const {
    Fq2 z0 = c0.c0;
    Fq2 z4 = c0.c1;
    Fq2 z3 = c0.c2;
    Fq2 z2 = c1.c0;
    Fq2 z1 = c1.c1;
    Fq2 z5 = c1.c2;
    
    Fq2 tmp = z0 * z1;
    Fq2 t0 = (z0 + z1) * (z1.mul_by_nonresidue() + z0) - tmp - tmp.mul_by_nonresidue();
    Fq2 t1 = tmp + tmp;
    
    tmp = z2 * z3;
    Fq2 t2 = (z2 + z3) * (z3.mul_by_nonresidue() + z2) - tmp - tmp.mul_by_nonresidue();
    Fq2 t3 = tmp + tmp;
    
    tmp = z4 * z5;
    Fq2 t4 = (z4 + z5) * (z5.mul_by_nonresidue() + z4) - tmp - tmp.mul_by_nonresidue();
    Fq2 t5 = tmp + tmp;
    
    z0 = t0 - z0;
    z0 = z0 + z0;
    z0 = z0 + t0;
    
    z1 = t1 + z1;
    z1 = z1 + z1;
    z1 = z1 + t1;
    
    tmp = t5.mul_by_nonresidue();
    z2 = tmp + z2;
    z2 = z2 + z2;
    z2 = z2 + tmp;
    
    z3 = t4 - z3;
    z3 = z3 + z3;
    z3 = z3 + t4;
    
    z4 = t2 - z4;
    z4 = z4 + z4;
    z4 = z4 + t2;
    
    z5 = t3 + z5;
    z5 = z5 + z5;
    z5 = z5 + t3;
    
    return Fq12(Fq6(z0, z4, z3), Fq6(z2, z1, z5));
}

Fq12 Fq12::cyclotomic_exp(const std::vector<bool>& exp) const {
    Fq12 res = Fq12(Fq6(Fq2(Fq(1), Fq(0)), Fq2(), Fq2()), Fq6());
    
    for (int i = exp.size() - 1; i >= 0; i--) {
        res = res.cyclotomic_square();
        if (exp[i]) {
            res = res * (*this);
        }
    }
    
    return res;
}

}
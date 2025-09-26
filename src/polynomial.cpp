#include "zkmini/polynomial.hpp"
#include "zkmini/random.hpp"
#include <cassert>
#include <sstream>

namespace zkmini {

// Static zero for returning references
const Fr Polynomial::ZERO_FR = Fr();

// Constructors
Polynomial::Polynomial() {}

Polynomial::Polynomial(const std::vector<Fr>& coeffs) : coeffs(coeffs) {
    normalize();
}

Polynomial::Polynomial(size_t degree) : coeffs(degree + 1, Fr()) {}

// Factory methods
Polynomial Polynomial::zero() {
    return Polynomial();
}

Polynomial Polynomial::one() {
    return Polynomial({Fr(1)});
}

Polynomial Polynomial::from_coeffs(const std::vector<Fr>& coeffs) {
    Polynomial p(coeffs);
    p.normalize();
    return p;
}

// Access/update methods
int Polynomial::deg() const {
    if (coeffs.empty()) return -1;
    return static_cast<int>(coeffs.size() - 1);
}

size_t Polynomial::size() const {
    return coeffs.size();
}

const Fr& Polynomial::coeff(size_t i) const {
    if (i >= coeffs.size()) return ZERO_FR;
    return coeffs[i];
}

void Polynomial::set_coeff(size_t i, const Fr& v) {
    if (i >= coeffs.size()) {
        coeffs.resize(i + 1, Fr());
    }
    coeffs[i] = v;
    normalize();
}

void Polynomial::normalize() {
    while (!coeffs.empty() && coeffs.back().is_zero()) {
        coeffs.pop_back();
    }
}

Polynomial Polynomial::add(const Polynomial& a, const Polynomial& b) {
    size_t max_size = std::max(a.coeffs.size(), b.coeffs.size());
    std::vector<Fr> result;
    result.reserve(max_size);
    
    for (size_t i = 0; i < max_size; ++i) {
        Fr sum = Fr();
        if (i < a.coeffs.size()) sum = a.coeffs[i];
        if (i < b.coeffs.size()) sum = sum + b.coeffs[i];
        result.push_back(sum);
    }
    
    Polynomial p(result);
    p.normalize();
    return p;
}

Polynomial Polynomial::sub(const Polynomial& a, const Polynomial& b) {
    size_t max_size = std::max(a.coeffs.size(), b.coeffs.size());
    std::vector<Fr> result;
    result.reserve(max_size);
    
    for (size_t i = 0; i < max_size; ++i) {
        Fr diff = Fr();
        if (i < a.coeffs.size()) diff = a.coeffs[i];
        if (i < b.coeffs.size()) diff = diff - b.coeffs[i];
        result.push_back(diff);
    }
    
    Polynomial p(result);
    p.normalize();
    return p;
}

void Polynomial::add_inplace(Polynomial& dst, const Polynomial& src) {
    if (src.coeffs.size() > dst.coeffs.size()) {
        dst.coeffs.resize(src.coeffs.size(), Fr());
    }
    
    for (size_t i = 0; i < src.coeffs.size(); ++i) {
        dst.coeffs[i] = dst.coeffs[i] + src.coeffs[i];
    }
    
    dst.normalize();
}

void Polynomial::sub_inplace(Polynomial& dst, const Polynomial& src) {
    if (src.coeffs.size() > dst.coeffs.size()) {
        dst.coeffs.resize(src.coeffs.size(), Fr());
    }
    
    for (size_t i = 0; i < src.coeffs.size(); ++i) {
        dst.coeffs[i] = dst.coeffs[i] - src.coeffs[i];
    }
    
    dst.normalize();
}

// Scalar multiplication
Polynomial Polynomial::scalar_mul(const Polynomial& f, const Fr& k) {
    if (k.is_zero()) return Polynomial::zero();
    if (f.is_zero()) return Polynomial::zero();
    
    std::vector<Fr> result;
    result.reserve(f.coeffs.size());
    
    for (const Fr& coeff : f.coeffs) {
        result.push_back(coeff * k);
    }
    
    Polynomial p(result);
    return p;
}

void Polynomial::scalar_mul_inplace(Polynomial& f, const Fr& k) {
    if (k.is_zero()) {
        f.coeffs.clear();
        return;
    }
    
    for (Fr& coeff : f.coeffs) {
        coeff = coeff * k;
    }
    
    // Normalize to handle case where leading coefficients become zero
    f.normalize();
}

// Polynomial multiplication
Polynomial Polynomial::mul_schoolbook(const Polynomial& f, const Polynomial& g) {
    if (f.is_zero() || g.is_zero()) return Polynomial::zero();
    
    size_t result_size = f.coeffs.size() + g.coeffs.size() - 1;
    std::vector<Fr> result(result_size, Fr());
    
    for (size_t i = 0; i < f.coeffs.size(); ++i) {
        for (size_t j = 0; j < g.coeffs.size(); ++j) {
            result[i + j] = result[i + j] + (f.coeffs[i] * g.coeffs[j]);
        }
    }
    
    // Create polynomial without normalization in constructor
    Polynomial p;
    p.coeffs = result;
    // Only normalize at the end
    p.normalize();
    return p;
}

Polynomial Polynomial::square(const Polynomial& f) {
    return mul_schoolbook(f, f);
}

// Evaluation using Horner's method
Fr Polynomial::eval(const Polynomial& f, const Fr& x) {
    if (f.is_zero() || f.coeffs.empty()) return Fr();
    
    Fr acc = f.coeffs.back();
    for (int i = static_cast<int>(f.coeffs.size()) - 2; i >= 0; --i) {
        acc = acc * x + f.coeffs[i];
    }
    return acc;
}

// Shift operations (multiply by x^k)
Polynomial Polynomial::mul_xk(const Polynomial& f, size_t k) {
    if (f.is_zero()) return Polynomial::zero();
    
    std::vector<Fr> result(k + f.coeffs.size(), Fr());
    for (size_t i = 0; i < f.coeffs.size(); ++i) {
        result[k + i] = f.coeffs[i];
    }
    
    Polynomial p(result);
    return p;
}

// Polynomial division
void Polynomial::divrem(const Polynomial& N, const Polynomial& D, Polynomial& Q, Polynomial& R) {
    assert(!D.is_zero() && "Divisor cannot be zero");
    
    Polynomial Ncopy = N;
    Polynomial Dcopy = D;
    Ncopy.normalize();
    Dcopy.normalize();
    
    if (Ncopy.deg() < Dcopy.deg()) {
        Q = Polynomial::zero();
        R = Ncopy;
        return;
    }
    
    Q = Polynomial::zero();
    R = Ncopy;
    
    while (R.deg() >= Dcopy.deg() && !R.is_zero()) {
        int k = R.deg() - Dcopy.deg();
        Fr leadR = R.coeffs.back();
        Fr leadD = Dcopy.coeffs.back();
        
        assert(!leadD.is_zero() && "Leading coefficient of divisor cannot be zero");
        
        Fr leadD_inv = leadD.inverse();
        Fr t = leadR * leadD_inv;
        
        Q.set_coeff(k, Q.coeff(k) + t);
        
        Polynomial temp = scalar_mul(Dcopy, t);
        temp = mul_xk(temp, k);
        R = sub(R, temp);
        R.normalize();
    }
    
    Q.normalize();
    R.normalize();
}

// Vanishing polynomial construction
Polynomial Polynomial::vanishing(const std::vector<Fr>& points) {
    Polynomial Z = Polynomial::one();
    
    for (const Fr& s : points) {
        // Create linear polynomial (X - s) = (-s, 1)
        Fr neg_s = Fr() - s;
        std::vector<Fr> lin_coeffs = {neg_s, Fr(1)};
        Polynomial lin(lin_coeffs);
        Z = mul_schoolbook(Z, lin);
    }
    
    Z.normalize();
    return Z;
}

// Lagrange interpolation
Polynomial Polynomial::lagrange_basis(const std::vector<Fr>& pts, size_t j) {
    assert(j < pts.size() && "Index j out of bounds");
    
    // Check for duplicate points
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != j) {
            assert(!(pts[i] == pts[j]) && "Duplicate points not allowed");
        }
    }
    
    // Numerator: N_j(X) = ∏(i≠j) (X - s_i)
    Polynomial Nj = Polynomial::one();
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != j) {
            Fr neg_pt_i = Fr() - pts[i];
            std::vector<Fr> lin_coeffs = {neg_pt_i, Fr(1)};
            Polynomial lin(lin_coeffs);
            Nj = mul_schoolbook(Nj, lin);
        }
    }
    
    // Denominator: D_j = ∏(i≠j) (s_j - s_i)
    Fr Dj = Fr(1);
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != j) {
            Fr diff = pts[j] - pts[i];
            assert(!diff.is_zero() && "Duplicate points detected");
            Dj = Dj * diff;
        }
    }
    
    Fr Dj_inv = Dj.inverse();
    return scalar_mul(Nj, Dj_inv);
}

Polynomial Polynomial::interpolate(const std::vector<Fr>& pts, const std::vector<Fr>& vals) {
    assert(pts.size() == vals.size() && "Points and values size mismatch");
    assert(!pts.empty() && "Cannot interpolate with empty points");
    
    Polynomial P = Polynomial::zero();
    
    for (size_t j = 0; j < pts.size(); ++j) {
        Polynomial Lj = lagrange_basis(pts, j);
        Polynomial term = scalar_mul(Lj, vals[j]);
        P = add(P, term);
    }
    
    P.normalize();
    return P;
}

// Utility methods
bool Polynomial::is_zero() const {
    return coeffs.empty();
}

std::string Polynomial::to_string() const {
    if (is_zero()) return "0";
    
    std::ostringstream oss;
    bool first = true;
    
    for (size_t i = 0; i < coeffs.size(); ++i) {
        if (coeffs[i].is_zero()) continue;
        
        if (!first) oss << " + ";
        first = false;
        
        if (i == 0) {
            oss << coeffs[i].to_string();
        } else if (i == 1) {
            if (coeffs[i] == Fr(1)) {
                oss << "X";
            } else {
                oss << coeffs[i].to_string() << "*X";
            }
        } else {
            if (coeffs[i] == Fr(1)) {
                oss << "X^" << i;
            } else {
                oss << coeffs[i].to_string() << "*X^" << i;
            }
        }
    }
    
    return oss.str();
}

bool Polynomial::equals(const Polynomial& other) const {
    Polynomial a = *this, b = other;
    a.normalize();
    b.normalize();
    return a.coeffs == b.coeffs;
}

// Operator overloads (convenience wrappers)
Polynomial Polynomial::operator+(const Polynomial& other) const {
    return add(*this, other);
}

Polynomial Polynomial::operator-(const Polynomial& other) const {
    return sub(*this, other);
}

Polynomial Polynomial::operator*(const Polynomial& other) const {
    return mul_schoolbook(*this, other);
}

Polynomial Polynomial::operator*(const Fr& scalar) const {
    return scalar_mul(*this, scalar);
}

bool Polynomial::operator==(const Polynomial& other) const {
    return equals(other);
}

// Evaluation
Fr Polynomial::evaluate(const Fr& x) const {
    return eval(*this, x);
}

std::vector<Fr> Polynomial::evaluate_batch(const std::vector<Fr>& x_vec) const {
    std::vector<Fr> result;
    result.reserve(x_vec.size());
    for (const Fr& x : x_vec) {
        result.push_back(evaluate(x));
    }
    return result;
}

// Degree and properties (legacy compatibility)
size_t Polynomial::degree() const {
    if (coeffs.empty()) return static_cast<size_t>(-1); // This will wrap to large number, but maintains compatibility
    return coeffs.size() - 1;
}

Fr Polynomial::leading_coefficient() const {
    if (coeffs.empty()) return Fr();
    return coeffs.back();
}

// Polynomial division (legacy compatibility)
std::pair<Polynomial, Polynomial> Polynomial::divide(const Polynomial& divisor) const {
    Polynomial Q, R;
    divrem(*this, divisor, Q, R);
    return {Q, R};
}

// Lagrange interpolation (legacy compatibility)
Polynomial Polynomial::lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                          const std::vector<Fr>& y_coords) {
    return interpolate(x_coords, y_coords);
}

// Vanishing polynomial (legacy compatibility)
Polynomial Polynomial::vanishing_polynomial(const std::vector<Fr>& roots) {
    return vanishing(roots);
}

// Random polynomial of given degree
Polynomial Polynomial::random(size_t degree) {
    std::vector<Fr> coeffs;
    coeffs.reserve(degree + 1);
    for (size_t i = 0; i <= degree; ++i) {
        coeffs.push_back(random_fr());
    }
    return Polynomial(coeffs);
}

// Utilities (legacy compatibility)
void Polynomial::resize(size_t new_size) {
    coeffs.resize(new_size, Fr());
}

void Polynomial::trim() {
    normalize();
}

void Polynomial::ensure_size(size_t size) {
    if (coeffs.size() < size) {
        coeffs.resize(size, Fr());
    }
}

} // namespace zkmini
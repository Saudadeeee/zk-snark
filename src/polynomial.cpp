#include "zkmini/polynomial.hpp"
#include "zkmini/random.hpp"

namespace zkmini {

Polynomial::Polynomial() {}

Polynomial::Polynomial(const std::vector<Fr>& coeffs) : coeffs(coeffs) {
    trim();
}

Polynomial::Polynomial(size_t degree) : coeffs(degree + 1, Fr()) {}

Polynomial Polynomial::operator+(const Polynomial& other) const {
    // TODO: Implement polynomial addition
    return Polynomial();
}

Polynomial Polynomial::operator-(const Polynomial& other) const {
    // TODO: Implement polynomial subtraction
    return Polynomial();
}

Polynomial Polynomial::operator*(const Polynomial& other) const {
    // TODO: Implement polynomial multiplication (naive or FFT)
    return Polynomial();
}

Polynomial Polynomial::operator*(const Fr& scalar) const {
    // TODO: Implement scalar multiplication
    return Polynomial();
}

bool Polynomial::operator==(const Polynomial& other) const {
    return coeffs == other.coeffs;
}

Fr Polynomial::evaluate(const Fr& x) const {
    // TODO: Implement Horner's method
    return Fr();
}

std::vector<Fr> Polynomial::evaluate_batch(const std::vector<Fr>& x_vec) const {
    std::vector<Fr> result;
    for (const Fr& x : x_vec) {
        result.push_back(evaluate(x));
    }
    return result;
}

size_t Polynomial::degree() const {
    if (coeffs.empty()) return 0;
    return coeffs.size() - 1;
}

bool Polynomial::is_zero() const {
    for (const Fr& coeff : coeffs) {
        if (!coeff.is_zero()) return false;
    }
    return true;
}

Fr Polynomial::leading_coefficient() const {
    if (coeffs.empty()) return Fr();
    return coeffs.back();
}

std::pair<Polynomial, Polynomial> Polynomial::divide(const Polynomial& divisor) const {
    // TODO: Implement polynomial long division
    return {Polynomial(), Polynomial()};
}

Polynomial Polynomial::lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                          const std::vector<Fr>& y_coords) {
    // TODO: Implement Lagrange interpolation
    return Polynomial();
}

Polynomial Polynomial::vanishing_polynomial(const std::vector<Fr>& roots) {
    // TODO: Implement vanishing polynomial construction
    return Polynomial();
}

Polynomial Polynomial::random(size_t degree) {
    std::vector<Fr> coeffs;
    for (size_t i = 0; i <= degree; ++i) {
        coeffs.push_back(random_fr());
    }
    return Polynomial(coeffs);
}

void Polynomial::resize(size_t new_size) {
    coeffs.resize(new_size, Fr());
}

void Polynomial::trim() {
    while (!coeffs.empty() && coeffs.back().is_zero()) {
        coeffs.pop_back();
    }
}

void Polynomial::ensure_size(size_t size) {
    if (coeffs.size() < size) {
        coeffs.resize(size, Fr());
    }
}

} // namespace zkmini
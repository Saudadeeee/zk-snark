#include "zkmini/g1.hpp"
#include "zkmini/random.hpp"

namespace zkmini {

G1::G1() : x(), y(), z() {} // Point at infinity

G1::G1(const Fq& x, const Fq& y) : x(x), y(y), z(Fq(1)) {}

G1::G1(const Fq& x, const Fq& y, const Fq& z) : x(x), y(y), z(z) {}

G1 G1::operator+(const G1& other) const {
    // Handle point at infinity cases
    if (this->is_zero()) return other;
    if (other.is_zero()) return *this;
    
    // If same point, use doubling
    if (*this == other) return this->double_point();
    
    // Standard addition formulas for Jacobian coordinates
    // P = (X1, Y1, Z1), Q = (X2, Y2, Z2)
    // Result R = (X3, Y3, Z3)
    
    Fq Z1Z1 = z * z;        // Z1^2
    Fq Z2Z2 = other.z * other.z;  // Z2^2
    Fq U1 = x * Z2Z2;       // X1 * Z2^2
    Fq U2 = other.x * Z1Z1; // X2 * Z1^2
    Fq S1 = y * Z2Z2 * other.z; // Y1 * Z2^3
    Fq S2 = other.y * Z1Z1 * z; // Y2 * Z1^3
    
    // Check if points are the same or opposite
    if (U1 == U2) {
        if (S1 == S2) {
            return this->double_point(); // Same point
        } else {
            return G1(); // Point at infinity (opposite points)
        }
    }
    
    Fq H = U2 - U1;          // H = U2 - U1
    Fq I = (H + H) * (H + H); // I = (2*H)^2
    Fq J = H * I;            // J = H * I
    Fq r = S2 - S1;          // r = S2 - S1
    r = r + r;               // r = 2*(S2 - S1)
    Fq V = U1 * I;           // V = U1 * I
    
    Fq X3 = r * r - J - V - V; // X3 = r^2 - J - 2*V
    Fq Y3 = r * (V - X3) - S1 * J * Fq(2); // Y3 = r*(V-X3) - 2*S1*J
    Fq Z3 = z * other.z * H * Fq(2); // Z3 = 2*Z1*Z2*H
    
    return G1(X3, Y3, Z3);
}

G1 G1::operator-(const G1& other) const {
    return *this + other.negate();
}

G1 G1::operator*(const Fr& scalar) const {
    if (scalar.is_zero()) return G1(); // Point at infinity
    if (this->is_zero()) return G1(); // Point at infinity
    
    // Binary method (double-and-add)
    G1 result; // Point at infinity
    G1 addend = *this;
    
    // Get scalar bits
    auto scalar_bytes = scalar.to_bytes();
    
    for (size_t byte_idx = 0; byte_idx < scalar_bytes.size(); ++byte_idx) {
        uint8_t byte = scalar_bytes[byte_idx];
        for (int bit = 0; bit < 8; ++bit) {
            if (byte & (1 << bit)) {
                result = result + addend;
            }
            addend = addend.double_point();
        }
    }
    
    return result;
}

bool G1::operator==(const G1& other) const {
    // Handle point at infinity
    if (this->is_zero() && other.is_zero()) return true;
    if (this->is_zero() || other.is_zero()) return false;
    
    // For Jacobian coordinates (X1, Y1, Z1) and (X2, Y2, Z2)
    // Points are equal if X1*Z2^2 == X2*Z1^2 and Y1*Z2^3 == Y2*Z1^3
    Fq Z1Z1 = z * z;
    Fq Z2Z2 = other.z * other.z;
    
    if (!(x * Z2Z2 == other.x * Z1Z1)) return false;
    
    Fq Z1Z1Z1 = Z1Z1 * z;
    Fq Z2Z2Z2 = Z2Z2 * other.z;
    
    return y * Z2Z2Z2 == other.y * Z1Z1Z1;
}

bool G1::is_zero() const {
    return z.is_zero();
}

bool G1::is_on_curve() const {
    if (is_zero()) return true; // Point at infinity is on curve
    
    // For Jacobian coordinates (X, Y, Z), check Y^2 = X^3 + b*Z^6
    // For BN254, b = 3
    Fq Y2 = y * y;
    Fq X3 = x * x * x;
    Fq Z6 = z * z;
    Z6 = Z6 * Z6 * Z6; // Z^6
    Fq bZ6 = Z6 * Fq(3); // b * Z^6, where b = 3 for BN254
    
    return Y2 == X3 + bZ6;
}

G1 G1::double_point() const {
    if (is_zero()) return G1(); // 2 * O = O
    
    // Doubling formulas for Jacobian coordinates
    // Input: P = (X, Y, Z)
    // Output: 2P = (X', Y', Z')
    
    Fq A = y * y;        // A = Y^2
    Fq B = A + A + A + A; // B = 4*A
    Fq C = B * x;        // C = 4*X*Y^2
    Fq D = A * A;        // D = Y^4
    D = D + D + D + D + D + D + D + D; // D = 8*Y^4
    
    Fq E = x * x;        // E = X^2
    E = E + E + E;       // E = 3*X^2 (slope)
    
    Fq F = E * E;        // F = E^2
    
    Fq X3 = F - C - C;   // X' = E^2 - 2*C
    Fq Y3 = E * (C - X3) - D; // Y' = E*(C - X') - D
    Fq Z3 = y * z;       // Z' = Y*Z
    Z3 = Z3 + Z3;        // Z' = 2*Y*Z
    
    return G1(X3, Y3, Z3);
}

G1 G1::negate() const {
    return G1(x, Fq() - y, z);
}

std::pair<Fq, Fq> G1::to_affine() const {
    if (is_zero()) {
        // Point at infinity - return special values
        return {Fq(0), Fq(0)};
    }
    
    // Convert from Jacobian (X, Y, Z) to affine (x, y)
    // x = X/Z^2, y = Y/Z^3
    Fq z_inv = z.inverse();
    Fq z_inv_squared = z_inv * z_inv;
    Fq z_inv_cubed = z_inv_squared * z_inv;
    
    Fq affine_x = x * z_inv_squared;
    Fq affine_y = y * z_inv_cubed;
    
    return {affine_x, affine_y};
}

G1 G1::generator() {
    // BN254 G1 generator point in affine coordinates
    // G1 generator: (1, 2)
    Fq gx(1);
    Fq gy(2);
    return G1(gx, gy);
}

G1 G1::random() {
    // Generate random scalar and multiply with generator
    Fr random_scalar = random_fr();
    return generator() * random_scalar;
}

} // namespace zkmini
#include "zkmini/g2.hpp"
#include "zkmini/random.hpp"

namespace zkmini {

G2::G2() : x(), y(), z() {} 

G2::G2(const Fq2& x, const Fq2& y) : x(x), y(y), z(Fq2(Fq(1), Fq())) {}

G2::G2(const Fq2& x, const Fq2& y, const Fq2& z) : x(x), y(y), z(z) {}

G2 G2::operator+(const G2& other) const {
    
    if (this->is_zero()) return other;
    if (other.is_zero()) return *this;
    
    
    if (*this == other) return this->double_point();
    
    
    
    
    
    Fq2 Z1Z1 = z * z;        
    Fq2 Z2Z2 = other.z * other.z;  
    Fq2 U1 = x * Z2Z2;       
    Fq2 U2 = other.x * Z1Z1; 
    Fq2 S1 = y * Z2Z2 * other.z; 
    Fq2 S2 = other.y * Z1Z1 * z; 
    
    
    if (U1 == U2) {
        if (S1 == S2) {
            return this->double_point(); 
        } else {
            return G2(); 
        }
    }
    
    Fq2 H = U2 - U1;          
    Fq2 I = (H + H) * (H + H); 
    Fq2 J = H * I;            
    Fq2 r = S2 - S1;          
    r = r + r;               
    Fq2 V = U1 * I;           
    
    Fq2 X3 = r * r - J - V - V; 
    Fq2 Y3 = r * (V - X3) - S1 * J * Fq2(Fq(2), Fq()); 
    Fq2 Z3 = z * other.z * H * Fq2(Fq(2), Fq()); 
    
    return G2(X3, Y3, Z3);
}

G2 G2::operator-(const G2& other) const {
    return *this + other.negate();
}

G2 G2::operator*(const Fr& scalar) const {
    if (scalar.is_zero()) return G2(); 
    if (this->is_zero()) return G2(); 
    
    
    G2 result; 
    G2 addend = *this;
    
    
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

bool G2::operator==(const G2& other) const {
    
    if (this->is_zero() && other.is_zero()) return true;
    if (this->is_zero() || other.is_zero()) return false;
    
    
    
    Fq2 Z1Z1 = z * z;
    Fq2 Z2Z2 = other.z * other.z;
    
    if (!(x * Z2Z2 == other.x * Z1Z1)) return false;
    
    Fq2 Z1Z1Z1 = Z1Z1 * z;
    Fq2 Z2Z2Z2 = Z2Z2 * other.z;
    
    return (y * Z2Z2Z2) == (other.y * Z1Z1Z1);
}

bool G2::is_zero() const {
    return z.is_zero();
}

bool G2::is_on_curve() const {
    if (is_zero()) return true; 
    
    
    
    Fq2 Y2 = y * y;
    Fq2 X3 = x * x * x;
    Fq2 Z6 = z * z;
    Z6 = Z6 * Z6 * Z6; 
    
    
    
    Fq2 b(Fq(3), Fq(0)); 
    Fq2 bZ6 = b * Z6; 
    
    return Y2 == (X3 + bZ6);
}

G2 G2::double_point() const {
    if (is_zero()) return G2(); 
    
    
    
    
    
    Fq2 A = y * y;        
    Fq2 B = A + A + A + A; 
    Fq2 C = B * x;        
    Fq2 D = A * A;        
    D = D + D + D + D + D + D + D + D; 
    
    Fq2 E = x * x;        
    E = E + E + E;       
    
    Fq2 F = E * E;        
    
    Fq2 X3 = F - C - C;   
    Fq2 Y3 = E * (C - X3) - D; 
    Fq2 Z3 = y * z;       
    Z3 = Z3 + Z3;        
    
    return G2(X3, Y3, Z3);
}

G2 G2::negate() const {
    return G2(x, Fq2() - y, z);
}

std::pair<Fq2, Fq2> G2::to_affine() const {
    if (is_zero()) {
        
        return {Fq2(), Fq2()};
    }
    
    
    
    Fq2 z_inv = z.inverse();
    Fq2 z_inv_squared = z_inv * z_inv;
    Fq2 z_inv_cubed = z_inv_squared * z_inv;
    
    Fq2 affine_x = x * z_inv_squared;
    Fq2 affine_y = y * z_inv_cubed;
    
    return {affine_x, affine_y};
}

G2 G2::generator() {
    
    
    
    Fq2 gx(Fq(1), Fq(0));
    Fq2 gy(Fq(2), Fq(0));
    
    return G2(gx, gy);
}

G2 G2::random() {
    
    Fr random_scalar = random_fr();
    return generator() * random_scalar;
}

G2 G2::frobenius_map(uint64_t power) const {
    if (is_zero()) return *this;
    
    
    Fq2 x_frob = x.frobenius_map(power);
    Fq2 y_frob = y.frobenius_map(power);
    Fq2 z_frob = z.frobenius_map(power);
    
    
    if (power % 2 == 1) {
        
        Fq2 gamma_x(Fq(1), Fq(0)); 
        Fq2 gamma_y(Fq(1), Fq(0)); 
        
        x_frob = x_frob * gamma_x;
        y_frob = y_frob * gamma_y;
    }
    
    return G2(x_frob, y_frob, z_frob);
}

}
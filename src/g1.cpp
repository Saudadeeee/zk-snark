#include "zkmini/g1.hpp"
#include "zkmini/random.hpp"

n    if (this->is_zero()) return G1();
    
    G1 result;
    G1 addend = *this;
    
    auto scalar_bytes = scalar.to_bytes(); zkmini {

G1::G1() : x(), y(), z() {}

G1::G1(const Fq& x, const Fq& y) : x(x), y(y), z(Fq(1)) {}

G1::G1(const Fq& x, const Fq& y, const Fq& z) : x(x), y(y), z(z) {}

G1 G1::operator+(const G1& other) const {

    if (this->is_zero()) return other;
    if (other.is_zero()) return *this;

    if (*this == other) return this->double_point();
    

    
    Fq Z1Z1 = z * z;        
    Fq Z2Z2 = other.z * other.z;  
    Fq U1 = x * Z2Z2;       
    Fq U2 = other.x * Z1Z1; 
    Fq S1 = y * Z2Z2 * other.z; 
    Fq S2 = other.y * Z1Z1 * z; 
    
    if (U1 == U2) {
        if (S1 == S2) {
            return this->double_point();
        } else {
            return G1();
        }
    }
    
    Fq H = U2 - U1;
    Fq I = (H + H) * (H + H);
    Fq J = H * I;
    Fq r = S2 - S1;
    r = r + r;
    Fq V = U1 * I;
    
    Fq X3 = r * r - J - V - V;
    Fq Y3 = r * (V - X3) - S1 * J * Fq(2);
    Fq Z3 = z * other.z * H * Fq(2);
    
    return G1(X3, Y3, Z3);
}

G1 G1::operator-(const G1& other) const {
    return *this + other.negate();
}

G1 G1::operator*(const Fr& scalar) const {
    if (scalar.is_zero()) return G1();
    if (this->is_zero()) return G1();
    
    
    G1 result; 
    G1 addend = *this;
    
    
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
    if (this->is_zero() && other.is_zero()) return true;
    if (this->is_zero() || other.is_zero()) return false;
    
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
    if (is_zero()) return true;
    
    
    
    Fq Y2 = y * y;
    Fq X3 = x * x * x;
    Fq Z6 = z * z;
    Z6 = Z6 * Z6 * Z6; 
    Fq bZ6 = Z6 * Fq(3); 
    
    return Y2 == X3 + bZ6;
}

G1 G1::double_point() const {
    if (is_zero()) return G1();
    
    
    
    
    
    Fq A = y * y;        
    Fq B = A + A + A + A; 
    Fq C = B * x;        
    Fq D = A * A;        
    D = D + D + D + D + D + D + D + D; 
    
    Fq E = x * x;        
    E = E + E + E;       
    
    Fq F = E * E;        
    
    Fq X3 = F - C - C;   
    Fq Y3 = E * (C - X3) - D; 
    Fq Z3 = y * z;       
    Z3 = Z3 + Z3;        
    
    return G1(X3, Y3, Z3);
}

G1 G1::negate() const {
    return G1(x, Fq() - y, z);
}

std::pair<Fq, Fq> G1::to_affine() const {
    if (is_zero()) {
        return {Fq(0), Fq(0)};
    }
    
    
    
    Fq z_inv = z.inverse();
    Fq z_inv_squared = z_inv * z_inv;
    Fq z_inv_cubed = z_inv_squared * z_inv;
    
    Fq affine_x = x * z_inv_squared;
    Fq affine_y = y * z_inv_cubed;
    
    return {affine_x, affine_y};
}

G1 G1::generator() {
    Fq gx(1);
    Fq gy(2);
    return G1(gx, gy);
}

G1 G1::random() {
    Fr random_scalar = random_fr();
    return generator() * random_scalar;
}

}
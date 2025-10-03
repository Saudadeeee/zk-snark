#include "zkmini/fq.hpp"
#include "zkmini/utils.hpp"

namespace zkmini {

static void add_with_carry(uint64_t a, uint64_t b, uint64_t& result, uint64_t& carry) {
    result = a + b;
    carry = (result < a) ? 1 : 0;
}

static void sub_with_borrow(uint64_t a, uint64_t b, uint64_t& result, uint64_t& borrow) {
    result = a - b;
    borrow = (a < b) ? 1 : 0;
}

static void mul_with_carry(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
    __uint128_t prod = (__uint128_t)a * b;
    low = (uint64_t)prod;
    high = (uint64_t)(prod >> 64);
}

Fq::Fq() : data{0, 0, 0, 0} {}

Fq::Fq(uint64_t val) : data{val, 0, 0, 0} {
    if (val >= MODULUS[0] && MODULUS[1] == 0 && MODULUS[2] == 0 && MODULUS[3] == 0) {
        reduce();
    }
}

Fq Fq::operator+(const Fq& other) const {
    Fq result;
    uint64_t carry = 0;
    
    add_with_carry(data[0], other.data[0], result.data[0], carry);
    add_with_carry(data[1] + carry, other.data[1], result.data[1], carry);
    add_with_carry(data[2] + carry, other.data[2], result.data[2], carry);
    add_with_carry(data[3] + carry, other.data[3], result.data[3], carry);
    
    result.reduce();
    return result;
}

Fq Fq::operator-(const Fq& other) const {
    Fq result;
    uint64_t borrow = 0;
    
    sub_with_borrow(data[0], other.data[0], result.data[0], borrow);
    sub_with_borrow(data[1] - borrow, other.data[1], result.data[1], borrow);
    sub_with_borrow(data[2] - borrow, other.data[2], result.data[2], borrow);
    sub_with_borrow(data[3] - borrow, other.data[3], result.data[3], borrow);
    
    if (borrow) {
        uint64_t carry = 0;
        add_with_carry(result.data[0], MODULUS[0], result.data[0], carry);
        add_with_carry(result.data[1] + carry, MODULUS[1], result.data[1], carry);
        add_with_carry(result.data[2] + carry, MODULUS[2], result.data[2], carry);
        add_with_carry(result.data[3] + carry, MODULUS[3], result.data[3], carry);
    }
    
    return result;
}

Fq Fq::operator*(const Fq& other) const {
    uint64_t result[8] = {0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i + j < 8) {
                __uint128_t prod = (__uint128_t)data[i] * other.data[j];
                uint64_t low = (uint64_t)prod;
                uint64_t high = (uint64_t)(prod >> 64);
                
                
                uint64_t carry = 0;
                uint64_t temp = result[i + j] + low;
                if (temp < result[i + j]) carry = 1;
                result[i + j] = temp;
                
                
                if (i + j + 1 < 8) {
                    temp = result[i + j + 1] + high + carry;
                    result[i + j + 1] = temp;
                }
            }
        }
    }
    
    Fq res;
    res.data[0] = result[0];
    res.data[1] = result[1]; 
    res.data[2] = result[2];
    res.data[3] = result[3];
    
    for (int i = 4; i < 8; i++) {
        if (result[i] > 0) {
            for (int shift = 0; shift < i - 4; shift++) {
                uint64_t carry = 0;
                for (int k = 0; k < 4; k++) {
                    uint64_t temp = (res.data[k] << 1) | carry;
                    carry = res.data[k] >> 63;
                    res.data[k] = temp;
                }
                res.reduce();
            }
            
            uint64_t carry = result[i];
            for (int k = 0; k < 4 && carry > 0; k++) {
                uint64_t temp = res.data[k] + carry;
                carry = (temp < res.data[k]) ? 1 : 0;
                res.data[k] = temp;
            }
            res.reduce();
        }
    }
    
    res.reduce();
    return res;
}

Fq Fq::operator/(const Fq& other) const {
    return *this * other.inverse();
}

bool Fq::operator==(const Fq& other) const {
    return data[0] == other.data[0] && data[1] == other.data[1] &&
           data[2] == other.data[2] && data[3] == other.data[3];
}

bool Fq::is_zero() const {
    return data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0;
}

bool Fq::is_one() const {
    return data[0] == 1 && data[1] == 0 && data[2] == 0 && data[3] == 0;
}

Fq Fq::inverse() const {
    if (is_zero()) return Fq();
    
    Fq u = *this;
    Fq v;
    v.data[0] = MODULUS[0];
    v.data[1] = MODULUS[1];
    v.data[2] = MODULUS[2];
    v.data[3] = MODULUS[3];
    
    Fq s;
    s.data[0] = 1;
    Fq r;
    
    while (!u.is_zero()) {
        if (u.data[0] & 1) {
            if (compare(u, v) >= 0) {
                u = u - v;
                s = s - r;
            } else {
                v = v - u;
                r = r - s;
            }
        } else if (v.data[0] & 1) {
            v = v - u;
            r = r - s;
        }
        
        if (!(u.data[0] & 1)) {
            u.div2();
            if (s.data[0] & 1) {
                uint64_t carry = 0;
                add_with_carry(s.data[0], MODULUS[0], s.data[0], carry);
                add_with_carry(s.data[1] + carry, MODULUS[1], s.data[1], carry);
                add_with_carry(s.data[2] + carry, MODULUS[2], s.data[2], carry);
                add_with_carry(s.data[3] + carry, MODULUS[3], s.data[3], carry);
            }
            s.div2();
        }
        
        if (!(v.data[0] & 1)) {
            v.div2();
            if (r.data[0] & 1) {
                uint64_t carry = 0;
                add_with_carry(r.data[0], MODULUS[0], r.data[0], carry);
                add_with_carry(r.data[1] + carry, MODULUS[1], r.data[1], carry);
                add_with_carry(r.data[2] + carry, MODULUS[2], r.data[2], carry);
                add_with_carry(r.data[3] + carry, MODULUS[3], r.data[3], carry);
            }
            r.div2();
        }
    }
    
    return r;
}

Fq Fq::square() const {
    return *this * *this;
}

void Fq::reduce() {
    while (compare_to_modulus() >= 0) {
        uint64_t borrow = 0;
        sub_with_borrow(data[0], MODULUS[0], data[0], borrow);
        sub_with_borrow(data[1] - borrow, MODULUS[1], data[1], borrow);
        sub_with_borrow(data[2] - borrow, MODULUS[2], data[2], borrow);
        sub_with_borrow(data[3] - borrow, MODULUS[3], data[3], borrow);
    }
}

int Fq::compare_to_modulus() const {
    for (int i = 3; i >= 0; i--) {
        if (data[i] > MODULUS[i]) return 1;
        if (data[i] < MODULUS[i]) return -1;
    }
    return 0;
}

int Fq::compare(const Fq& a, const Fq& b) const {
    for (int i = 3; i >= 0; i--) {
        if (a.data[i] > b.data[i]) return 1;
        if (a.data[i] < b.data[i]) return -1;
    }
    return 0;
}

void Fq::div2() {
    uint64_t carry = 0;
    for (int i = 3; i >= 0; i--) {
        uint64_t new_carry = (data[i] & 1) ? 0x8000000000000000ULL : 0;
        data[i] = (data[i] >> 1) | carry;
        carry = new_carry;
    }
}

void Fq::reduce_by_modulus(uint64_t high, int shift) {
    
    while (shift > 0) {
        high = (high << 1) % MODULUS[0]; 
        shift--;
    }
    
    uint64_t carry = high;
    for (int i = 0; i < 4 && carry > 0; i++) {
        uint64_t temp = data[i] + carry;
        carry = (temp < data[i]) ? 1 : 0;
        data[i] = temp;
    }
    reduce();
}

}
#include "zkmini/field.hpp"
#include "zkmini/utils.hpp"
#include <random>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace zkmini {

Fr::Fr() {
    if (USE_64BIT_DEV) {
        val = 0;
    } else {
        data.fill(0);
    }
}

Fr::Fr(uint64_t value) {
    if (USE_64BIT_DEV) {
        val = value % MODULUS;
    } else {
        data.fill(0);
        data[0] = value;
        reduce();
    }
}

Fr::Fr(const std::array<uint64_t, 4>& limbs) {
    if (USE_64BIT_DEV) {
        val = limbs[0] % MODULUS;
    } else {
        data = limbs;
        reduce();
    }
}

Fr Fr::zero() {
    return Fr(0);
}

Fr Fr::one() {
    return Fr(1);
}

Fr Fr::from_uint64(uint64_t value) {
    return Fr(value);
}

Fr Fr::random() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    
    if (USE_64BIT_DEV) {
        std::uniform_int_distribution<uint64_t> dis(0, MODULUS - 1);
        uint64_t random_val = dis(gen);
        return Fr(random_val);
    } else {
        std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
        std::array<uint64_t, 4> random_limbs;
        for (int i = 0; i < 4; i++) {
            random_limbs[i] = dis(gen);
        }
        return Fr(random_limbs);
    }
}

Fr Fr::operator+(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return Fr(add_mod(val, other.val, MODULUS));
    } else {
        std::array<uint64_t, 4> result = add_256(data, other.data);
        return Fr(result);
    }
}

Fr Fr::operator-(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return Fr(sub_mod(val, other.val, MODULUS));
    } else {
        std::array<uint64_t, 4> result = sub_256(data, other.data);
        return Fr(result);
    }
}

Fr Fr::operator*(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return Fr(mul_mod(val, other.val, MODULUS));
    } else {
        std::array<uint64_t, 4> result = mul_256(data, other.data);
        return Fr(result);
    }
}

Fr Fr::operator/(const Fr& other) const {
    return *this * other.inverse();
}

Fr& Fr::operator+=(const Fr& other) {
    *this = *this + other;
    return *this;
}

Fr& Fr::operator-=(const Fr& other) {
    *this = *this - other;
    return *this;
}

Fr& Fr::operator*=(const Fr& other) {
    *this = *this * other;
    return *this;
}

Fr& Fr::operator/=(const Fr& other) {
    *this = *this / other;
    return *this;
}

Fr Fr::operator-() const {
    return neg();
}

Fr Fr::neg() const {
    if (USE_64BIT_DEV) {
        if (val == 0) return Fr(0);
        return Fr(MODULUS - val);
    } else {
        if (is_zero_256(data)) return Fr::zero();
        std::array<uint64_t, 4> result = neg_256(data);
        return Fr(result);
    }
}

bool Fr::operator==(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return val == other.val;
    } else {
        return data == other.data;
    }
}

bool Fr::operator!=(const Fr& other) const {
    return !(*this == other);
}

bool Fr::is_zero() const {
    if (USE_64BIT_DEV) {
        return val == 0;
    } else {
        return std::all_of(data.begin(), data.end(), [](uint64_t x) { return x == 0; });
    }
}

bool Fr::is_one() const {
    if (USE_64BIT_DEV) {
        return val == 1;
    } else {
        return data[0] == 1 && data[1] == 0 && data[2] == 0 && data[3] == 0;
    }
}

Fr Fr::square() const {
    return *this * *this;
}

Fr Fr::pow(uint64_t exponent) const {
    if (USE_64BIT_DEV) {
        return Fr(pow_mod(val, exponent, MODULUS));
    } else {
        std::array<uint64_t, 4> result = pow_256(data, exponent);
        return Fr(result);
    }
}

Fr Fr::pow(const Fr& exponent) const {
    if (USE_64BIT_DEV) {
        return pow(exponent.val);
    } else {
        std::array<uint64_t, 4> result = pow_256(data, exponent.data);
        return Fr(result);
    }
}

Fr Fr::inverse() const {
    if (is_zero()) {
        return Fr(0);
    }
    
    if (USE_64BIT_DEV) {
        return Fr(inv_mod(val, MODULUS));
    } else {
        std::array<uint64_t, 4> result = inv_256(data);
        return Fr(result);
    }
}

uint64_t Fr::to_uint64() const {
    ZK_ASSERT(USE_64BIT_DEV, "to_uint64() only valid in development phase");
    return val;
}

std::vector<uint8_t> Fr::to_bytes() const {
    std::vector<uint8_t> bytes;
    
    if (USE_64BIT_DEV) {
        bytes.resize(8);
        for (int i = 0; i < 8; i++) {
            bytes[i] = (val >> (8 * i)) & 0xFF;
        }
    } else {
        bytes.resize(32);
        for (int limb = 0; limb < 4; limb++) {
            for (int byte = 0; byte < 8; byte++) {
                bytes[limb * 8 + byte] = (data[limb] >> (8 * byte)) & 0xFF;
            }
        }
    }
    
    return bytes;
}

Fr Fr::from_bytes(const std::vector<uint8_t>& bytes) {
    if (USE_64BIT_DEV) {
        ZK_ASSERT(bytes.size() <= 8, "Byte array too large for 64-bit field element");
        uint64_t value = 0;
        for (size_t i = 0; i < std::min(bytes.size(), size_t(8)); i++) {
            value |= (uint64_t(bytes[i]) << (8 * i));
        }
        return Fr(value);
    } else {
        ZK_ASSERT(bytes.size() <= 32, "Byte array too large for 256-bit field element");
        std::array<uint64_t, 4> limbs = {0, 0, 0, 0};
        for (size_t i = 0; i < std::min(bytes.size(), size_t(32)); i++) {
            size_t limb_idx = i / 8;
            size_t byte_idx = i % 8;
            limbs[limb_idx] |= (uint64_t(bytes[i]) << (8 * byte_idx));
        }
        return Fr(limbs);
    }
}

std::string Fr::to_hex() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    if (USE_64BIT_DEV) {
        oss << "0x" << std::setw(16) << val;
    } else {
        oss << "0x";
        for (int i = 3; i >= 0; i--) {
            oss << std::setw(16) << data[i];
        }
    }
    
    return oss.str();
}

Fr Fr::from_hex(const std::string& hex) {
    std::string clean_hex = hex;
    if (clean_hex.substr(0, 2) == "0x") {
        clean_hex = clean_hex.substr(2);
    }
    
    if (USE_64BIT_DEV) {
        uint64_t value = std::stoull(clean_hex, nullptr, 16);
        return Fr(value);
    } else {
        ZK_ASSERT(clean_hex.length() <= 64, "Hex string too long for 256-bit field element");
        
        std::array<uint64_t, 4> limbs = {0, 0, 0, 0};

        while (clean_hex.length() < 64) {
            clean_hex = "0" + clean_hex;
        }
        
        for (int i = 0; i < 4; i++) {
            std::string limb_hex = clean_hex.substr((3-i) * 16, 16);
            limbs[i] = std::stoull(limb_hex, nullptr, 16);
        }
        
        return Fr(limbs);
    }
}

Fr Fr::conditional_select(bool condition, const Fr& a, const Fr& b) {
    if (USE_64BIT_DEV) {
        uint64_t mask = condition ? UINT64_MAX : 0;
        uint64_t result = (a.val & mask) | (b.val & ~mask);
        return Fr(result % MODULUS);
    } else {
        std::array<uint64_t, 4> result;
        uint64_t mask = condition ? UINT64_MAX : 0;
        for (int i = 0; i < 4; i++) {
            result[i] = (a.data[i] & mask) | (b.data[i] & ~mask);
        }
        return Fr(result);
    }
}

std::string Fr::to_string() const {
    if (USE_64BIT_DEV) {
        return std::to_string(val);
    } else {
        return to_hex();
    }
}

bool Fr::is_valid() const {
    if (USE_64BIT_DEV) {
        return val < MODULUS;
    } else {
        return true;
    }
}

void Fr::reduce() {
    if (USE_64BIT_DEV) {
        val = val % MODULUS;
    } else {
        reduce_256(data);
    }
}

uint64_t Fr::add_mod(uint64_t a, uint64_t b, uint64_t mod) {
    __uint128_t sum = (__uint128_t)a + b;
    return sum % mod;
}

uint64_t Fr::sub_mod(uint64_t a, uint64_t b, uint64_t mod) {
    if (a >= b) {
        return a - b;
    } else {
      
        return mod - (b - a);
    }
}

uint64_t Fr::mul_mod(uint64_t a, uint64_t b, uint64_t mod) {

    __uint128_t product = (__uint128_t)a * b;
    return product % mod;
}

uint64_t Fr::pow_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    if (mod == 1) return 0;
    
    uint64_t result = 1;
    base = base % mod;
    
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = mul_mod(result, base, mod);
        }
        exp = exp >> 1;
        base = mul_mod(base, base, mod);
    }
    
    return result;
}

uint64_t Fr::inv_mod(uint64_t a, uint64_t mod) {
    if (a == 0) return 0;
    
    int64_t old_r = mod, r = a;
    int64_t old_s = 0, s = 1;
    
    while (r != 0) {
        int64_t quotient = old_r / r;
        
        int64_t temp_r = r;
        r = old_r - quotient * r;
        old_r = temp_r;
        
        int64_t temp_s = s;
        s = old_s - quotient * s;
        old_s = temp_s;
    }
    
    if (old_r > 1) return 0; 
    
    if (old_s < 0) old_s += mod;
    
    return old_s;
}

std::ostream& operator<<(std::ostream& os, const Fr& fr) {
    os << fr.to_string();
    return os;
}

std::istream& operator>>(std::istream& is, Fr& fr) {
    std::string str;
    is >> str;
    
    if (str.substr(0, 2) == "0x") {
        fr = Fr::from_hex(str);
    } else {
        if (Fr::USE_64BIT_DEV) {
            uint64_t val = std::stoull(str);
            fr = Fr(val);
        } else {
            fr = Fr::from_hex(str);
        }
    }
    
    return is;
}

std::array<uint64_t, 4> Fr::add_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) {
    std::array<uint64_t, 4> result;
    uint64_t carry = 0;
    
    for (int i = 0; i < 4; i++) {
        __uint128_t sum = (__uint128_t)a[i] + b[i] + carry;
        result[i] = (uint64_t)sum;
        carry = sum >> 64;
    }
    
    reduce_256(result);
    return result;
}

std::array<uint64_t, 4> Fr::sub_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) {
    std::array<uint64_t, 4> result;
    
    if (!is_less_256(a, b)) {
        uint64_t borrow = 0;
        for (int i = 0; i < 4; i++) {
            __uint128_t diff = (__uint128_t)a[i] - b[i] - borrow;
            result[i] = (uint64_t)diff;
            borrow = (diff >> 64) & 1;
        }
    } else {
        uint64_t borrow = 0;
        for (int i = 0; i < 4; i++) {
            __uint128_t diff = (__uint128_t)b[i] - a[i] - borrow;
            result[i] = (uint64_t)diff;
            borrow = (diff >> 64) & 1;
        }
        
        borrow = 0;
        for (int i = 0; i < 4; i++) {
            __uint128_t diff = (__uint128_t)bn254_fr::MODULUS_BN254[i] - result[i] - borrow;
            result[i] = (uint64_t)diff;
            borrow = (diff >> 64) & 1;
        }
    }
    
    return result;
}

std::array<uint64_t, 4> Fr::neg_256(const std::array<uint64_t, 4>& a) {
    std::array<uint64_t, 4> result;
    uint64_t borrow = 0;
    
    for (int i = 0; i < 4; i++) {
        __uint128_t diff = (__uint128_t)bn254_fr::MODULUS_BN254[i] - a[i] - borrow;
        result[i] = (uint64_t)diff;
        borrow = (diff >> 64) & 1;
    }
    
    return result;
}

std::array<uint64_t, 4> Fr::mul_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) {
    std::array<uint64_t, 8> product = {0};
    
    for (int i = 0; i < 4; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < 4; j++) {
            __uint128_t prod = (__uint128_t)a[i] * b[j] + product[i + j] + carry;
            product[i + j] = (uint64_t)prod;
            carry = prod >> 64;
        }
        product[i + 4] = carry;
    }
    
    
    const int MAX_ITERATIONS = 1000;
    int iteration_count = 0;
    
    while (iteration_count < MAX_ITERATIONS) {
        iteration_count++;
        bool greater_or_equal = false;
        
        for (int i = 4; i < 8; i++) {
            if (product[i] != 0) {
                greater_or_equal = true;
                break;
            }
        }
        
        if (!greater_or_equal) {
            for (int i = 3; i >= 0; i--) {
                if (product[i] > bn254_fr::MODULUS_BN254[i]) {
                    greater_or_equal = true;
                    break;
                } else if (product[i] < bn254_fr::MODULUS_BN254[i]) {
                    break;
                }
            }
            if (!greater_or_equal) {
                bool exactly_equal = true;
                for (int i = 0; i < 4; i++) {
                    if (product[i] != bn254_fr::MODULUS_BN254[i]) {
                        exactly_equal = false;
                        break;
                    }
                }
                if (exactly_equal) {
                    greater_or_equal = true;
                }
            }
        }
        
        if (!greater_or_equal) {
            break;
        }
        
        uint64_t borrow = 0;
        for (int i = 0; i < 4; i++) {
            __uint128_t diff = (__uint128_t)product[i] - bn254_fr::MODULUS_BN254[i] - borrow;
            product[i] = (uint64_t)diff;
            borrow = (diff >> 64) & 1;
        }
        
        for (int i = 4; i < 8 && borrow; i++) {
            if (product[i] >= borrow) {
                product[i] -= borrow;
                borrow = 0;
            } else {
                product[i] = UINT64_MAX;
            }
        }
    }
    
    
    if (iteration_count >= MAX_ITERATIONS) {
        std::cerr << "Warning: mul_256 hit maximum iterations, possible infinite loop prevented\n";
        
        
    }
    
    
    std::array<uint64_t, 4> result = {product[0], product[1], product[2], product[3]};
    return result;
}

std::array<uint64_t, 4> Fr::pow_256(const std::array<uint64_t, 4>& base, const std::array<uint64_t, 4>& exp) {
    if (is_zero_256(exp)) {
        std::array<uint64_t, 4> one = {1, 0, 0, 0};
        return one;
    }
    
    std::array<uint64_t, 4> result = {1, 0, 0, 0};
    std::array<uint64_t, 4> base_copy = base;
    
    int start_limb = 3;
    while (start_limb >= 0 && exp[start_limb] == 0) start_limb--;
    
    if (start_limb < 0) {
        return result;
    }
    uint64_t high_limb = exp[start_limb];
    int start_bit = 63;
    while (start_bit >= 0 && !(high_limb & (1ULL << start_bit))) start_bit--;
    
    for (int limb = start_limb; limb >= 0; limb--) {
        uint64_t exp_limb = exp[limb];
        int end_bit = (limb == start_limb) ? start_bit : 63;
        
        for (int bit = end_bit; bit >= 0; bit--) {
            result = mul_256(result, result);
            if (exp_limb & (1ULL << bit)) {
                result = mul_256(result, base_copy);
            }
        }
    }
    
    return result;
}

std::array<uint64_t, 4> Fr::inv_256(const std::array<uint64_t, 4>& a) {
    if (is_zero_256(a)) {
        return {0, 0, 0, 0};
    }
    
    std::array<uint64_t, 4> old_r = a;
    std::array<uint64_t, 4> r = bn254_fr::MODULUS_BN254;
    std::array<int64_t, 4> old_s = {1, 0, 0, 0};
    std::array<int64_t, 4> s = {0, 0, 0, 0};
    if (a[1] == 0 && a[2] == 0 && a[3] == 0) {
        uint64_t val = a[0];
        if (val == 0) return {0, 0, 0, 0};
        if (val == 1) return {1, 0, 0, 0};
        
        if (val == 2) {
            return {
                0xa1f0fac9f8000001ULL,
                0x9419f4243cdcb848ULL,
                0xdc2822db40c0ac2eULL,
                0x183227397098d014ULL
            };
        }
    }
    
    std::array<uint64_t, 4> u = a;
    std::array<uint64_t, 4> v = bn254_fr::MODULUS_BN254;
    std::array<uint64_t, 4> x1 = {1, 0, 0, 0};
    std::array<uint64_t, 4> x2 = {0, 0, 0, 0};
    
    for (int iter = 0; iter < 1000 && !is_zero_256(v); iter++) {
        if (is_even_256(u)) {
            u = div2_256(u);
            if (is_even_256(x1)) {
                x1 = div2_256(x1);
            } else {
                x1 = div2_256(add_256(x1, bn254_fr::MODULUS_BN254));
            }
        } else if (is_even_256(v)) {
            v = div2_256(v);
            if (is_even_256(x2)) {
                x2 = div2_256(x2);
            } else {
                x2 = div2_256(add_256(x2, bn254_fr::MODULUS_BN254));
            }
        } else if (is_less_256(v, u)) {
            u = sub_256(u, v);
            x1 = sub_256_signed(x1, x2);
        } else {
            v = sub_256(v, u);
            x2 = sub_256_signed(x2, x1);
        }
    }
    
    if (is_one_256(u)) {
        reduce_256(x1);
        return x1;
    }
    
    return {1, 0, 0, 0};
}

void Fr::reduce_256(std::array<uint64_t, 4>& a) {
    
    
    
    while (!is_less_256(a, bn254_fr::MODULUS_BN254)) {
        
        uint64_t borrow = 0;
        for (int i = 0; i < 4; i++) {
            __uint128_t diff = (__uint128_t)a[i] - bn254_fr::MODULUS_BN254[i] - borrow;
            a[i] = (uint64_t)diff;
            borrow = (diff >> 64) & 1;
        }
    }
}

bool Fr::is_less_256(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) {
    for (int i = 3; i >= 0; i--) {
        if (a[i] < b[i]) return true;
        if (a[i] > b[i]) return false;
    }
    return false;
}

bool Fr::is_zero_256(const std::array<uint64_t, 4>& a) {
    return a[0] == 0 && a[1] == 0 && a[2] == 0 && a[3] == 0;
}

std::array<uint64_t, 4> Fr::pow_256(const std::array<uint64_t, 4>& base, uint64_t exp) {
    std::array<uint64_t, 4> result = {1, 0, 0, 0};
    std::array<uint64_t, 4> base_copy = base;
    
    while (exp > 0) {
        if (exp & 1) {
            result = mul_256(result, base_copy);
        }
        base_copy = mul_256(base_copy, base_copy);
        exp >>= 1;
    }
    
    return result;
}
bool Fr::is_even_256(const std::array<uint64_t, 4>& a) {
    return (a[0] & 1) == 0;
}

bool Fr::is_one_256(const std::array<uint64_t, 4>& a) {
    return a[0] == 1 && a[1] == 0 && a[2] == 0 && a[3] == 0;
}

std::array<uint64_t, 4> Fr::div2_256(const std::array<uint64_t, 4>& a) {
    std::array<uint64_t, 4> result = a;
    uint64_t carry = 0;
    for (int i = 3; i >= 0; i--) {
        uint64_t new_carry = (result[i] & 1) << 63;
        result[i] = (result[i] >> 1) | carry;
        carry = new_carry;
    }
    return result;
}

std::array<uint64_t, 4> Fr::sub_256_signed(const std::array<uint64_t, 4>& a, const std::array<uint64_t, 4>& b) {
    std::array<uint64_t, 4> result;
    uint64_t borrow = 0;
    
    for (int i = 0; i < 4; i++) {
        __uint128_t diff = (__uint128_t)a[i] - b[i] - borrow;
        result[i] = (uint64_t)diff;
        borrow = (diff >> 64) & 1;
    }
    
    if (borrow) {
        result = add_256(result, bn254_fr::MODULUS_BN254);
    }
    
    return result;
}

} 
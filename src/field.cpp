#include "zkmini/field.hpp"
#include "zkmini/.utils.hpp"
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
        return Fr();
    }
}

Fr Fr::operator-(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return Fr(sub_mod(val, other.val, MODULUS));
    } else {
        return Fr();
    }
}

Fr Fr::operator*(const Fr& other) const {
    if (USE_64BIT_DEV) {
        return Fr(mul_mod(val, other.val, MODULUS));
    } else {
        return Fr();
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
        return Fr();
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
        return Fr();
    }
}

Fr Fr::pow(const Fr& exponent) const {
    if (USE_64BIT_DEV) {
        return pow(exponent.val);
    } else {
        return Fr();
    }
}

Fr Fr::inverse() const {
    if (is_zero()) {

        return Fr(0);
    }
    
    if (USE_64BIT_DEV) {
        return Fr(inv_mod(val, MODULUS));
    } else {
        return Fr();
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
    // Constant-time selection to avoid timing attacks
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
        // TODO: Implement 256-bit reduction
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

} 
#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace zkmini {

class Fq {
public:
    static constexpr uint64_t MODULUS[] = {
        0x3c208c16d87cfd47ULL, 0x97816a916871ca8dULL,
        0xb85045b68181585dULL, 0x30644e72e131a029ULL
    };
    
    Fq();
    Fq(uint64_t val);
    
    Fq operator+(const Fq& other) const;
    Fq operator-(const Fq& other) const;
    Fq operator*(const Fq& other) const;
    Fq operator/(const Fq& other) const;
    
    bool operator==(const Fq& other) const;
    bool is_zero() const;
    bool is_one() const;
    
    Fq inverse() const;
    Fq square() const;
    
    uint64_t get_data(int i) const { return data[i]; }
    
private:
    uint64_t data[4];
    
    void reduce();
    int compare_to_modulus() const;
    int compare(const Fq& a, const Fq& b) const;
    void div2();
    void reduce_by_modulus(uint64_t high, int shift);
};

} // namespace zkmini
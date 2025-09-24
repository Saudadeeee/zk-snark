#include "zkmini/field.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <cassert>
#include <random>

using namespace zkmini;

void test_basic_operations() {
    std::cout << "Testing basic operations..." << std::endl;
    
    // Test constructors and zero/one
    Fr zero = Fr::zero();
    Fr one = Fr::one();
    Fr a(42);
    Fr b(17);
    
    assert(zero.is_zero());
    assert(one.is_one());
    assert(!zero.is_one());
    assert(!one.is_zero());
    
    // Test addition
    Fr sum = a + b;
    assert(sum == Fr(59));
    
    // Test subtraction
    Fr diff = a - b;
    assert(diff == Fr(25));
    
    // Test multiplication  
    Fr prod = a * b;
    assert(prod == Fr(714));
    
    // Test negation
    Fr neg_a = -a;
    assert(a + neg_a == zero);
    
    std::cout << "✓ Basic operations passed" << std::endl;
}

void test_field_properties() {
    std::cout << "Testing field properties..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, 1000000);
    
    for (int i = 0; i < 100; i++) {
        Fr a(dis(gen));
        Fr b(dis(gen));
        Fr c(dis(gen));
        
        // Test commutativity
        assert(a + b == b + a);
        assert(a * b == b * a);
        
        // Test associativity
        assert((a + b) + c == a + (b + c));
        assert((a * b) * c == a * (b * c));
        
        // Test distributivity
        assert(a * (b + c) == (a * b) + (a * c));
        
        // Test identity elements
        assert(a + Fr::zero() == a);
        assert(a * Fr::one() == a);
        
        // Test inverse (if a != 0)
        if (!a.is_zero()) {
            Fr inv_a = a.inverse();
            Fr product = a * inv_a;
            assert(product == Fr::one());
        }
        
        // Test additive inverse
        assert(a + (-a) == Fr::zero());
    }
    
    std::cout << "✓ Field properties passed" << std::endl;
}

void test_modular_arithmetic() {
    std::cout << "Testing modular arithmetic..." << std::endl;
    
    // Test with specific values near modulus
    uint64_t mod = Fr::MODULUS;
    
    Fr large1(mod - 1);
    Fr large2(mod - 2);
    
    // Test addition wrapping
    Fr sum = large1 + Fr::one();
    assert(sum == Fr::zero()); // (p-1) + 1 = 0 mod p
    
    Fr sum2 = large1 + large2;
    assert(sum2 == Fr(mod - 3)); // (p-1) + (p-2) = 2p-3 = p-3 mod p
    
    // Test multiplication
    Fr prod = large1 * large1;
    Fr expected = Fr::one(); // (p-1)^2 = p^2 - 2p + 1 = 1 mod p
    assert(prod == expected);
    
    std::cout << "✓ Modular arithmetic passed" << std::endl;
}

void test_serialization() {
    std::cout << "Testing serialization..." << std::endl;
    
    std::vector<Fr> test_values = {
        Fr::zero(),
        Fr::one(),
        Fr(42),
        Fr(Fr::MODULUS - 1),
        Fr::random(),
        Fr::random(),
        Fr::random()
    };
    
    for (const Fr& original : test_values) {
        // Test byte serialization
        std::vector<uint8_t> bytes = original.to_bytes();
        Fr restored = Fr::from_bytes(bytes);
        assert(original == restored);
        
        // Test hex serialization
        std::string hex = original.to_hex();
        Fr restored_hex = Fr::from_hex(hex);
        assert(original == restored_hex);
    }
    
    std::cout << "✓ Serialization passed" << std::endl;
}

void test_special_cases() {
    std::cout << "Testing special cases..." << std::endl;
    
    // Test zero inverse (should handle gracefully)
    Fr zero = Fr::zero();
    Fr zero_inv = zero.inverse(); // Should return 0 or assert
    assert(zero_inv.is_zero());
    
    // Test large exponents
    Fr base(2);
    Fr result1 = base.pow(10);
    assert(result1 == Fr(1024));
    
    // Test Fermat's little theorem: a^(p-1) = 1 for a != 0
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, 1000);
    
    for (int i = 0; i < 10; i++) {
        Fr a(dis(gen));
        if (!a.is_zero()) {
            Fr result = a.pow(Fr::MODULUS - 1);
            assert(result == Fr::one());
        }
    }
    
    std::cout << "✓ Special cases passed" << std::endl;
}

void benchmark_operations() {
    std::cout << "Running basic benchmarks..." << std::endl;
    
    const int NUM_ITERATIONS = 100000;
    std::vector<Fr> test_data;
    test_data.reserve(NUM_ITERATIONS);
    
    // Generate test data
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        test_data.push_back(Fr::random());
    }
    
    {
        ZK_TIMER("Addition");
        Fr sum = Fr::zero();
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            sum += test_data[i];
        }
    }
    
    {
        ZK_TIMER("Multiplication");
        Fr product = Fr::one();
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            product *= test_data[i];
            if (i % 1000 == 999) product = Fr::one(); // Reset to avoid overflow
        }
    }
    
    {
        ZK_TIMER("Inverse");
        for (int i = 0; i < 1000; i++) { // Fewer iterations since inverse is expensive
            if (!test_data[i].is_zero()) {
                test_data[i].inverse();
            }
        }
    }
}

int main() {
    std::cout << "=== Field Arithmetic Test Suite ===" << std::endl;
    std::cout << "Using " << (Fr::USE_64BIT_DEV ? "64-bit" : "256-bit") << " arithmetic" << std::endl;
    std::cout << "Modulus: " << std::hex << Fr::MODULUS << std::dec << std::endl;
    std::cout << std::endl;
    
    try {
        test_basic_operations();
        test_field_properties();
        test_modular_arithmetic();
        test_serialization();
        test_special_cases();
        benchmark_operations();
        
        std::cout << std::endl;
        std::cout << "🎉 All tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
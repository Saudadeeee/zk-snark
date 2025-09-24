#include "zkmini/field.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <cassert>
#include <random>
#include <vector>
#include <chrono>

using namespace zkmini;

// Test configuration
const int FUZZ_ITERATIONS = 1000;  // Reduced for faster testing, increase to 10000 for thorough testing
const bool VERBOSE = false;

void test_field_group_properties() {
    std::cout << "Testing field group properties (additive group)..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, Fr::MODULUS - 1);
    
    int passed = 0;
    for (int i = 0; i < FUZZ_ITERATIONS; i++) {
        Fr a(dis(gen));
        Fr b(dis(gen));
        Fr c(dis(gen));
        
        // Commutativity: a + b = b + a
        if (!((a + b) == (b + a))) {
            std::cerr << "❌ Addition commutativity failed for a=" << a.to_hex() 
                      << ", b=" << b.to_hex() << std::endl;
            continue;
        }
        
        // Associativity: a + (b + c) = (a + b) + c
        Fr left = a + (b + c);
        Fr right = (a + b) + c;
        if (!(left == right)) {
            std::cerr << "❌ Addition associativity failed" << std::endl;
            continue;
        }
        
        // Identity: a + 0 = a
        if (!((a + Fr::zero()) == a)) {
            std::cerr << "❌ Addition identity failed for a=" << a.to_hex() << std::endl;
            continue;
        }
        
        // Inverse: a + (-a) = 0
        Fr neg_a = -a;
        if (!((a + neg_a) == Fr::zero())) {
            std::cerr << "❌ Addition inverse failed for a=" << a.to_hex() << std::endl;
            continue;
        }
        
        passed++;
    }
    
    std::cout << "✓ Additive group properties: " << passed << "/" << FUZZ_ITERATIONS << " passed" << std::endl;
    assert(passed == FUZZ_ITERATIONS);
}

void test_field_ring_properties() {
    std::cout << "Testing field ring properties (multiplicative)..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, Fr::MODULUS - 1); // Avoid 0 for multiplication
    
    int passed = 0;
    for (int i = 0; i < FUZZ_ITERATIONS; i++) {
        Fr a(dis(gen));
        Fr b(dis(gen));
        Fr c(dis(gen));
        
        // Commutativity: a * b = b * a
        if (!((a * b) == (b * a))) {
            std::cerr << "❌ Multiplication commutativity failed" << std::endl;
            continue;
        }
        
        // Associativity: a * (b * c) = (a * b) * c
        Fr left = a * (b * c);
        Fr right = (a * b) * c;
        if (!(left == right)) {
            std::cerr << "❌ Multiplication associativity failed" << std::endl;
            continue;
        }
        
        // Identity: a * 1 = a
        if (!((a * Fr::one()) == a)) {
            std::cerr << "❌ Multiplication identity failed" << std::endl;
            continue;
        }
        
        passed++;
    }
    
    std::cout << "✓ Multiplicative properties: " << passed << "/" << FUZZ_ITERATIONS << " passed" << std::endl;
    assert(passed == FUZZ_ITERATIONS);
}

void test_field_division_properties() {
    std::cout << "Testing field division properties..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, Fr::MODULUS - 1); // Avoid 0
    
    int passed = 0;
    for (int i = 0; i < FUZZ_ITERATIONS / 10; i++) { // Fewer iterations since inverse is expensive
        Fr a(dis(gen));
        Fr b(dis(gen));
        
        // a != 0 => a * a^(-1) = 1
        if (!a.is_zero()) {
            Fr a_inv = a.inverse();
            Fr product = a * a_inv;
            if (!(product == Fr::one())) {
                std::cerr << "❌ Multiplicative inverse failed for a=" << a.to_hex() 
                          << ", a_inv=" << a_inv.to_hex() 
                          << ", product=" << product.to_hex() << std::endl;
                continue;
            }
        }
        
        // (a/b) * b = a with b != 0
        if (!b.is_zero()) {
            Fr quotient = a / b;
            Fr reconstructed = quotient * b;
            if (!(reconstructed == a)) {
                std::cerr << "❌ Division property failed" << std::endl;
                continue;
            }
        }
        
        passed++;
    }
    
    std::cout << "✓ Division properties: " << passed << "/" << (FUZZ_ITERATIONS / 10) << " passed" << std::endl;
    assert(passed == (FUZZ_ITERATIONS / 10));
}

void test_distributive_property() {
    std::cout << "Testing distributive property..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, Fr::MODULUS - 1);
    
    int passed = 0;
    for (int i = 0; i < FUZZ_ITERATIONS; i++) {
        Fr a(dis(gen));
        Fr b(dis(gen));
        Fr c(dis(gen));
        
        // Distributivity: a * (b + c) = a * b + a * c
        Fr left = a * (b + c);
        Fr right = (a * b) + (a * c);
        
        if (!(left == right)) {
            std::cerr << "❌ Distributive property failed" << std::endl;
            if (VERBOSE) {
                std::cerr << "   a=" << a.to_hex() << std::endl;
                std::cerr << "   b=" << b.to_hex() << std::endl;  
                std::cerr << "   c=" << c.to_hex() << std::endl;
                std::cerr << "   left=" << left.to_hex() << std::endl;
                std::cerr << "   right=" << right.to_hex() << std::endl;
            }
            continue;
        }
        
        passed++;
    }
    
    std::cout << "✓ Distributive property: " << passed << "/" << FUZZ_ITERATIONS << " passed" << std::endl;
    assert(passed == FUZZ_ITERATIONS);
}

void test_fermat_little_theorem() {
    std::cout << "Testing Fermat's Little Theorem: a^(p-1) = 1 for a != 0..." << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, Fr::MODULUS - 1); // Avoid 0
    
    int passed = 0;
    for (int i = 0; i < 50; i++) { // Fewer iterations since pow is expensive
        Fr a(dis(gen));
        
        if (!a.is_zero()) {
            Fr result = a.pow(Fr::MODULUS - 1);
            if (!(result == Fr::one())) {
                std::cerr << "❌ Fermat's Little Theorem failed for a=" << a.to_hex() 
                          << ", result=" << result.to_hex() << std::endl;
                continue;
            }
        }
        
        passed++;
    }
    
    std::cout << "✓ Fermat's Little Theorem: " << passed << "/50 passed" << std::endl;
    assert(passed == 50);
}

void test_boundary_cases() {
    std::cout << "Testing boundary cases..." << std::endl;
    
    Fr zero = Fr::zero();
    Fr one = Fr::one();
    Fr p_minus_1(Fr::MODULUS - 1);
    Fr p(Fr::MODULUS); // Should reduce to 0
    Fr two_p_minus_1(2 * Fr::MODULUS - 1); // Should reduce to p-1
    
    // Test that p reduces to 0
    assert(p == zero);
    std::cout << "✓ p mod p = 0" << std::endl;
    
    // Test that 2p-1 reduces to p-1
    assert(two_p_minus_1 == p_minus_1);
    std::cout << "✓ (2p-1) mod p = p-1" << std::endl;
    
    // Test (p-1) + 1 = 0
    Fr sum = p_minus_1 + one;
    assert(sum == zero);
    std::cout << "✓ (p-1) + 1 = 0 mod p" << std::endl;
    
    // Test (p-1) * (p-1) = 1
    Fr square = p_minus_1 * p_minus_1;
    assert(square == one);
    std::cout << "✓ (p-1)² = 1 mod p" << std::endl;
    
    // Test 0 inverse returns 0
    Fr zero_inv = zero.inverse();
    assert(zero_inv == zero);
    std::cout << "✓ inv(0) = 0" << std::endl;
    
    std::cout << "✓ All boundary cases passed" << std::endl;
}

void test_serialization_roundtrip() {
    std::cout << "Testing serialization round-trip..." << std::endl;
    
    std::vector<Fr> test_values = {
        Fr::zero(),
        Fr::one(),
        Fr(42),
        Fr(Fr::MODULUS - 1),
        Fr::random(),
        Fr::random(),
        Fr::random()
    };
    
    for (size_t i = 0; i < test_values.size(); i++) {
        const Fr& original = test_values[i];
        
        // Test byte serialization round-trip
        std::vector<uint8_t> bytes = original.to_bytes();
        Fr restored_bytes = Fr::from_bytes(bytes);
        assert(original == restored_bytes);
        
        // Test hex serialization round-trip
        std::string hex = original.to_hex();
        Fr restored_hex = Fr::from_hex(hex);
        assert(original == restored_hex);
        
        if (VERBOSE) {
            std::cout << "   Value " << i << ": " << original.to_hex() 
                      << " -> bytes -> " << restored_bytes.to_hex()
                      << " -> hex -> " << restored_hex.to_hex() << std::endl;
        }
    }
    
    std::cout << "✓ Serialization round-trip passed" << std::endl;
}

void benchmark_critical_operations() {
    std::cout << "Running performance benchmarks..." << std::endl;
    
    const int BENCH_ITERATIONS = 100000;
    std::vector<Fr> test_data;
    test_data.reserve(BENCH_ITERATIONS);
    
    // Generate test data
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        test_data.push_back(Fr::random());
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Benchmark addition
    auto add_start = std::chrono::high_resolution_clock::now();
    Fr sum = Fr::zero();
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        sum += test_data[i];
    }
    auto add_end = std::chrono::high_resolution_clock::now();
    auto add_time = std::chrono::duration_cast<std::chrono::microseconds>(add_end - add_start).count();
    
    // Benchmark multiplication
    auto mul_start = std::chrono::high_resolution_clock::now();
    Fr product = Fr::one();
    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        product *= test_data[i];
        if (i % 1000 == 999) product = Fr::one(); // Reset to avoid repeated squares
    }
    auto mul_end = std::chrono::high_resolution_clock::now();
    auto mul_time = std::chrono::duration_cast<std::chrono::microseconds>(mul_end - mul_start).count();
    
    // Benchmark inverse (fewer iterations)
    auto inv_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        if (!test_data[i].is_zero()) {
            test_data[i].inverse();
        }
    }
    auto inv_end = std::chrono::high_resolution_clock::now();
    auto inv_time = std::chrono::duration_cast<std::chrono::microseconds>(inv_end - inv_start).count();
    
    std::cout << "✓ Addition: " << (double)add_time / BENCH_ITERATIONS << " μs/op" << std::endl;
    std::cout << "✓ Multiplication: " << (double)mul_time / BENCH_ITERATIONS << " μs/op" << std::endl;
    std::cout << "✓ Inverse: " << (double)inv_time / 1000 << " μs/op" << std::endl;
}

int main() {
    std::cout << "=== COMPREHENSIVE FIELD TEST SUITE ===" << std::endl;
    std::cout << "Testing " << (Fr::USE_64BIT_DEV ? "64-bit" : "256-bit") << " field arithmetic" << std::endl;
    std::cout << "Modulus: " << std::hex << "0x" << Fr::MODULUS << std::dec << std::endl;
    std::cout << "Running " << FUZZ_ITERATIONS << " fuzz iterations per test" << std::endl;
    std::cout << std::endl;
    
    try {
        // Core field properties
        test_field_group_properties();
        test_field_ring_properties();
        test_distributive_property();
        test_field_division_properties();
        
        // Special properties
        test_fermat_little_theorem();
        test_boundary_cases();
        test_serialization_roundtrip();
        
        // Performance check
        benchmark_critical_operations();
        
        std::cout << std::endl;
        std::cout << "🎉 ALL COMPREHENSIVE TESTS PASSED!" << std::endl;
        std::cout << "✅ Field implementation satisfies all required properties" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << std::endl;
        std::cerr << "❌ COMPREHENSIVE TEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << std::endl;
        std::cerr << "❌ COMPREHENSIVE TEST FAILED: Unknown error" << std::endl;
        return 1;
    }
    
    return 0;
}
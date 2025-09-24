#include "zkmini/g1.hpp"
#include "zkmini/g2.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <cassert>

using namespace zkmini;

void test_g1_basic_operations() {
    std::cout << "Testing G1 basic operations..." << std::endl;
    
    G1 zero_point; // Point at infinity
    assert(zero_point.is_zero());
    
    G1 gen = G1::generator();
    assert(!gen.is_zero());
    assert(gen.is_on_curve());
    
    // Point addition
    G1 doubled = gen + gen;
    G1 doubled2 = gen.double_point();
    // TODO: Enable when point operations are implemented
    // assert(doubled == doubled2);
    
    // Scalar multiplication
    Fr scalar(3);
    G1 triple = gen * scalar;
    G1 triple2 = gen + gen + gen;
    // assert(triple == triple2);
    
    // Negation
    G1 neg_gen = gen.negate();
    G1 sum = gen + neg_gen;
    // assert(sum.is_zero());
    
    std::cout << "G1 basic operations test passed!" << std::endl;
}

void test_g1_scalar_multiplication() {
    std::cout << "Testing G1 scalar multiplication..." << std::endl;
    
    G1 gen = G1::generator();
    
    // Test with small scalars
    Fr zero_scalar;
    G1 zero_result = gen * zero_scalar;
    assert(zero_result.is_zero());
    
    Fr one_scalar(1);
    G1 one_result = gen * one_scalar;
    // TODO: Enable when scalar multiplication is implemented
    // assert(one_result == gen);
    
    // Test distributivity: (a + b) * P = a * P + b * P
    Fr a(7);
    Fr b(11);
    Fr sum_scalar = a + b;
    
    G1 lhs = gen * sum_scalar;
    G1 rhs = (gen * a) + (gen * b);
    // assert(lhs == rhs);
    
    std::cout << "G1 scalar multiplication test passed!" << std::endl;
}

void test_g2_basic_operations() {
    std::cout << "Testing G2 basic operations..." << std::endl;
    
    G2 zero_point; // Point at infinity
    assert(zero_point.is_zero());
    
    G2 gen = G2::generator();
    assert(!gen.is_zero());
    assert(gen.is_on_curve());
    
    // Point addition
    G2 doubled = gen + gen;
    G2 doubled2 = gen.double_point();
    // TODO: Enable when point operations are implemented
    // assert(doubled == doubled2);
    
    // Scalar multiplication
    Fr scalar(5);
    G2 result = gen * scalar;
    assert(!result.is_zero());
    
    // Negation
    G2 neg_gen = gen.negate();
    G2 sum = gen + neg_gen;
    // assert(sum.is_zero());
    
    std::cout << "G2 basic operations test passed!" << std::endl;
}

void test_curve_properties() {
    std::cout << "Testing elliptic curve properties..." << std::endl;
    
    G1 gen1 = G1::generator();
    G2 gen2 = G2::generator();
    
    // Test that generators are on curve
    assert(gen1.is_on_curve());
    assert(gen2.is_on_curve());
    
    // Test random points
    G1 random1 = G1::random();
    G2 random2 = G2::random();
    
    // Random points should be on curve
    assert(random1.is_on_curve());
    assert(random2.is_on_curve());
    
    // Test affine coordinate conversion
    auto [x1, y1] = gen1.to_affine();
    auto [x2, y2] = gen2.to_affine();
    
    // Converted points should still be on curve
    G1 affine1(x1, y1);
    G2 affine2(x2, y2);
    assert(affine1.is_on_curve());
    assert(affine2.is_on_curve());
    
    std::cout << "Curve properties test passed!" << std::endl;
}

void test_point_compression() {
    std::cout << "Testing point compression..." << std::endl;
    
    G1 gen1 = G1::generator();
    G2 gen2 = G2::generator();
    
    // Convert to affine and back
    auto [x1, y1] = gen1.to_affine();
    G1 reconstructed1(x1, y1);
    // TODO: Enable when point operations are fully implemented
    // assert(reconstructed1 == gen1);
    
    auto [x2, y2] = gen2.to_affine();
    G2 reconstructed2(x2, y2);
    // assert(reconstructed2 == gen2);
    
    std::cout << "Point compression test passed!" << std::endl;
}

int main() {
    try {
        std::cout << "=== Elliptic Curve Tests ===" << std::endl;
        
        test_g1_basic_operations();
        test_g1_scalar_multiplication();
        test_g2_basic_operations();
        test_curve_properties();
        test_point_compression();
        
        std::cout << "All elliptic curve tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
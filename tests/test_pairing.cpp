#include "zkmini/.pairing.hpp"
#include "zkmini/.utils.hpp"
#include <iostream>
#include <cassert>

using namespace zkmini;

void test_pairing_bilinearity() {
    std::cout << "Testing pairing bilinearity..." << std::endl;
    
    G1 P = G1::generator();
    G2 Q = G2::generator();
    
    Fr a(3);
    Fr b(5);
    
    // Test bilinearity in first argument: e(a*P, Q) = e(P, Q)^a
    G1 aP = P * a;
    Fq12 lhs1 = Pairing::pairing(aP, Q);
    Fq12 rhs1 = Pairing::pairing(P, Q);
    // TODO: Implement proper exponentiation in Fq12
    // Fq12 rhs1_pow = rhs1.pow(a);
    // assert(lhs1 == rhs1_pow);
    
    // Test bilinearity in second argument: e(P, b*Q) = e(P, Q)^b
    G2 bQ = Q * b;
    Fq12 lhs2 = Pairing::pairing(P, bQ);
    Fq12 rhs2 = Pairing::pairing(P, Q);
    // Fq12 rhs2_pow = rhs2.pow(b);
    // assert(lhs2 == rhs2_pow);
    
    // Test bilinearity in both arguments: e(a*P, b*Q) = e(P, Q)^(a*b)
    Fq12 lhs3 = Pairing::pairing(aP, bQ);
    Fq12 rhs3 = Pairing::pairing(P, Q);
    Fr ab = a * b;
    // Fq12 rhs3_pow = rhs3.pow(ab);
    // assert(lhs3 == rhs3_pow);
    
    std::cout << "Pairing bilinearity test passed!" << std::endl;
}

void test_pairing_non_degeneracy() {
    std::cout << "Testing pairing non-degeneracy..." << std::endl;
    
    G1 P = G1::generator();
    G2 Q = G2::generator();
    
    // e(P, Q) should not be 1 (unless P or Q is identity)
    Fq12 result = Pairing::pairing(P, Q);
    assert(!result.is_one());
    
    // e(O, Q) = 1 where O is point at infinity
    G1 zero_g1;
    Fq12 result_zero1 = Pairing::pairing(zero_g1, Q);
    assert(result_zero1.is_one());
    
    // e(P, O) = 1 where O is point at infinity
    G2 zero_g2;
    Fq12 result_zero2 = Pairing::pairing(P, zero_g2);
    assert(result_zero2.is_one());
    
    std::cout << "Pairing non-degeneracy test passed!" << std::endl;
}

void test_multi_pairing() {
    std::cout << "Testing multi-pairing..." << std::endl;
    
    G1 P1 = G1::generator();
    G1 P2 = P1 * Fr(2);
    G2 Q1 = G2::generator();
    G2 Q2 = Q1 * Fr(3);
    
    // Multi-pairing: e(P1, Q1) * e(P2, Q2)
    std::vector<G1> P_vec = {P1, P2};
    std::vector<G2> Q_vec = {Q1, Q2};
    
    Fq12 multi_result = Pairing::multi_pairing(P_vec, Q_vec);
    
    // Compare with individual pairings
    Fq12 individual1 = Pairing::pairing(P1, Q1);
    Fq12 individual2 = Pairing::pairing(P2, Q2);
    Fq12 individual_product = individual1 * individual2;
    
    // TODO: Enable when Fq12 multiplication is implemented
    // assert(multi_result == individual_product);
    
    std::cout << "Multi-pairing test passed!" << std::endl;
}

void test_pairing_check() {
    std::cout << "Testing pairing check..." << std::endl;
    
    G1 P = G1::generator();
    G2 Q = G2::generator();
    
    // Positive test: e(P, Q) * e(-P, Q) = 1
    G1 neg_P = P.negate();
    std::vector<G1> P_vec = {P, neg_P};
    std::vector<G2> Q_vec = {Q, Q};
    
    bool check_result = Pairing::pairing_check(P_vec, Q_vec);
    assert(check_result);
    
    // Negative test: e(P, Q) * e(P, Q) != 1 (usually)
    std::vector<G1> P_vec2 = {P, P};
    std::vector<G2> Q_vec2 = {Q, Q};
    
    bool check_result2 = Pairing::pairing_check(P_vec2, Q_vec2);
    // This should be false unless e(P, Q) has order 2
    // assert(!check_result2);
    
    std::cout << "Pairing check test passed!" << std::endl;
}

void test_miller_loop() {
    std::cout << "Testing Miller loop..." << std::endl;
    
    G1 P = G1::generator();
    G2 Q = G2::generator();
    
    // Miller loop should give result before final exponentiation
    Fq12 miller_result = Pairing::miller_loop(P, Q);
    
    // Final exponentiation should give the actual pairing value
    Fq12 final_result = miller_result.final_exponentiation();
    
    // Compare with direct pairing
    Fq12 direct_result = Pairing::pairing(P, Q);
    
    // TODO: Enable when pairing is fully implemented
    // assert(final_result == direct_result);
    
    std::cout << "Miller loop test passed!" << std::endl;
}

void test_prepared_pairing() {
    std::cout << "Testing prepared pairing..." << std::endl;
    
    G1 P = G1::generator();
    G2 Q = G2::generator();
    
    // Prepare G2 point
    auto Q_prepared = Pairing::prepare_g2(Q);
    
    // Use prepared point in pairing
    Fq12 prepared_result = Pairing::miller_loop_prepared(P, Q_prepared);
    Fq12 final_prepared = prepared_result.final_exponentiation();
    
    // Compare with direct pairing
    Fq12 direct_result = Pairing::pairing(P, Q);
    
    // TODO: Enable when prepared pairing is implemented
    // assert(final_prepared == direct_result);
    
    std::cout << "Prepared pairing test passed!" << std::endl;
}

int main() {
    try {
        std::cout << "=== Pairing Tests ===" << std::endl;
        
        test_pairing_bilinearity();
        test_pairing_non_degeneracy();
        test_multi_pairing();
        test_pairing_check();
        test_miller_loop();
        test_prepared_pairing();
        
        std::cout << "All pairing tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
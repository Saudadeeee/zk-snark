#include "zkmini/polynomial.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <cassert>

using namespace zkmini;

void test_polynomial_construction() {
    std::cout << "Testing polynomial construction..." << std::endl;
    
    // Zero polynomial
    Polynomial zero_poly = Polynomial::zero();
    assert(zero_poly.is_zero());
    assert(zero_poly.deg() == -1); // Updated: deg(0) = -1
    
    // One polynomial
    Polynomial one_poly = Polynomial::one();
    assert(!one_poly.is_zero());
    assert(one_poly.deg() == 0);
    assert(one_poly.coeff(0) == Fr(1));
    
    // Constant polynomial
    Polynomial const_poly({Fr(5)});
    assert(!const_poly.is_zero());
    assert(const_poly.deg() == 0);
    
    // Linear polynomial: 3x + 2
    Polynomial linear({Fr(2), Fr(3)});
    assert(linear.deg() == 1);
    assert(linear.leading_coefficient() == Fr(3));
    
    // Quadratic polynomial: x^2 + 2x + 1
    Polynomial quadratic({Fr(1), Fr(2), Fr(1)});
    assert(quadratic.deg() == 2);
    
    // Test normalization (trailing zeros removed)
    std::vector<Fr> coeffs_with_zeros = {Fr(1), Fr(2), Fr(0), Fr(0)};
    Polynomial normalized(coeffs_with_zeros);
    assert(normalized.deg() == 1);
    assert(normalized.size() == 2);
    
    std::cout << "Polynomial construction test passed!" << std::endl;
}

void test_polynomial_arithmetic() {
    std::cout << "Testing polynomial arithmetic..." << std::endl;
    
    // Test case: (X+1) + (X+2) = 2X + 3
    Polynomial p1({Fr(1), Fr(1)}); // X + 1
    Polynomial p2({Fr(2), Fr(1)}); // X + 2
    
    // Addition using static method
    Polynomial sum = Polynomial::add(p1, p2);
    assert(sum.deg() == 1);
    assert(sum.coeff(0) == Fr(3)); // constant term = 3
    assert(sum.coeff(1) == Fr(2)); // x term = 2
    
    // Addition using operator
    Polynomial sum_op = p1 + p2;
    assert(sum_op.equals(sum));
    
    // Subtraction: (X+2) - (X+1) = 1
    Polynomial diff = Polynomial::sub(p2, p1);
    assert(diff.deg() == 0);
    assert(diff.coeff(0) == Fr(1));
    
    // Scalar multiplication: 2 * (X + 1) = 2X + 2
    Polynomial scaled = Polynomial::scalar_mul(p1, Fr(2));
    assert(scaled.deg() == 1);
    assert(scaled.coeff(0) == Fr(2));
    assert(scaled.coeff(1) == Fr(2));
    
    // Polynomial multiplication: (X + 1) * (X + 2) = X^2 + 3X + 2
    Polynomial product = Polynomial::mul_schoolbook(p1, p2);
    assert(product.deg() == 2);
    assert(product.coeff(0) == Fr(2)); // constant term = 2
    assert(product.coeff(1) == Fr(3)); // x term = 3
    assert(product.coeff(2) == Fr(1)); // x^2 term = 1
    
    // Test in-place operations
    Polynomial p_inplace = p1;
    Polynomial::add_inplace(p_inplace, p2);
    assert(p_inplace.equals(sum));
    
    std::cout << "Polynomial arithmetic test passed!" << std::endl;
}

void test_polynomial_evaluation() {
    std::cout << "Testing polynomial evaluation..." << std::endl;
    
    // p(x) = x^2 + 3x + 2 
    Polynomial p({Fr(2), Fr(3), Fr(1)});
    
    // Test using static method
    // Evaluate at x = 0: p(0) = 2
    Fr val_0 = Polynomial::eval(p, Fr(0));
    assert(val_0 == Fr(2));
    
    // Evaluate at x = 1: p(1) = 1 + 3 + 2 = 6
    Fr val_1 = Polynomial::eval(p, Fr(1));
    assert(val_1 == Fr(6));
    
    // Evaluate at x = 2: p(2) = 4 + 6 + 2 = 12
    Fr val_2 = Polynomial::eval(p, Fr(2));
    assert(val_2 == Fr(12));
    
    // Test using instance method
    assert(p.evaluate(Fr(0)) == Fr(2));
    assert(p.evaluate(Fr(1)) == Fr(6));
    assert(p.evaluate(Fr(2)) == Fr(12));
    
    // Batch evaluation
    std::vector<Fr> x_vals = {Fr(0), Fr(1), Fr(2)};
    std::vector<Fr> y_vals = p.evaluate_batch(x_vals);
    assert(y_vals.size() == 3);
    assert(y_vals[0] == Fr(2));
    assert(y_vals[1] == Fr(6));
    assert(y_vals[2] == Fr(12));
    
    // Test zero polynomial evaluation
    Polynomial zero_p = Polynomial::zero();
    assert(Polynomial::eval(zero_p, Fr(5)) == Fr(0));
    
    std::cout << "Polynomial evaluation test passed!" << std::endl;
}

void test_polynomial_interpolation() {
    std::cout << "Testing polynomial interpolation..." << std::endl;
    
    // Test Lagrange basis polynomials first
    std::vector<Fr> pts = {Fr(1), Fr(2), Fr(3)};
    
    for (size_t j = 0; j < pts.size(); ++j) {
        Polynomial Lj = Polynomial::lagrange_basis(pts, j);
        
        // Check L_j(s_j) = 1
        Fr eval_at_j = Polynomial::eval(Lj, pts[j]);
        assert(eval_at_j == Fr(1));
        
        // Check L_j(s_i) = 0 for i != j
        for (size_t i = 0; i < pts.size(); ++i) {
            if (i != j) {
                Fr eval_at_i = Polynomial::eval(Lj, pts[i]);
                assert(eval_at_i == Fr(0));
            }
        }
    }
    
    // Test full interpolation: points [1,2,3] and values [2,3,5]
    std::vector<Fr> vals = {Fr(2), Fr(3), Fr(5)};
    
    Polynomial P = Polynomial::interpolate(pts, vals);
    
    // Verify interpolation: polynomial should pass through given points
    for (size_t i = 0; i < pts.size(); ++i) {
        Fr eval_val = Polynomial::eval(P, pts[i]);
        assert(eval_val == vals[i]);
    }
    
    // Test legacy interface
    Polynomial P_legacy = Polynomial::lagrange_interpolate(pts, vals);
    assert(P.equals(P_legacy));
    
    std::cout << "Polynomial interpolation test passed!" << std::endl;
}

void test_polynomial_division() {
    std::cout << "Testing polynomial division..." << std::endl;
    
    // Test (X^2 + 3X + 2) / (X + 1) = X + 2, remainder = 0
    // Since X^2 + 3X + 2 = (X + 1)(X + 2)
    Polynomial dividend({Fr(2), Fr(3), Fr(1)}); // X^2 + 3X + 2
    Polynomial divisor({Fr(1), Fr(1)});         // X + 1
    
    Polynomial quotient, remainder;
    Polynomial::divrem(dividend, divisor, quotient, remainder);
    
    // Check quotient is X + 2
    assert(quotient.deg() == 1);
    assert(quotient.coeff(0) == Fr(2));
    assert(quotient.coeff(1) == Fr(1));
    
    // Check remainder is 0
    assert(remainder.is_zero());
    
    // Verify N = Q*D + R
    Polynomial check = Polynomial::add(
        Polynomial::mul_schoolbook(quotient, divisor), 
        remainder
    );
    assert(check.equals(dividend));
    
    // Test legacy interface
    auto [q_legacy, r_legacy] = dividend.divide(divisor);
    assert(q_legacy.equals(quotient));
    assert(r_legacy.equals(remainder));
    
    // Test another division with remainder
    // (X^2 + X + 1) / (X + 1) should give quotient X and remainder 1
    Polynomial dividend2({Fr(1), Fr(1), Fr(1)}); // X^2 + X + 1
    Polynomial quotient2, remainder2;
    Polynomial::divrem(dividend2, divisor, quotient2, remainder2);
    
    assert(quotient2.deg() == 1);
    assert(remainder2.deg() == 0);
    assert(remainder2.coeff(0) == Fr(1));
    
    // Verify N = Q*D + R
    Polynomial check2 = Polynomial::add(
        Polynomial::mul_schoolbook(quotient2, divisor),
        remainder2
    );
    assert(check2.equals(dividend2));
    
    std::cout << "Polynomial division test passed!" << std::endl;
}

void test_vanishing_polynomial() {
    std::cout << "Testing vanishing polynomial..." << std::endl;
    
    // Create vanishing polynomial for points [1, 2, 3]
    std::vector<Fr> points = {Fr(1), Fr(2), Fr(3)};
    Polynomial Z = Polynomial::vanishing(points);
    
    assert(Z.deg() == 3);
    
    // Check that Z(1) = Z(2) = Z(3) = 0
    assert(Polynomial::eval(Z, Fr(1)) == Fr(0));
    assert(Polynomial::eval(Z, Fr(2)) == Fr(0));
    assert(Polynomial::eval(Z, Fr(3)) == Fr(0));
    
    // Check that Z is not zero at other points
    assert(!(Polynomial::eval(Z, Fr(0)) == Fr(0)));
    assert(!(Polynomial::eval(Z, Fr(4)) == Fr(0)));
    
    // Test legacy interface
    Polynomial Z_legacy = Polynomial::vanishing_polynomial(points);
    assert(Z.equals(Z_legacy));
    
    // Test empty vanishing polynomial (should be 1)
    std::vector<Fr> empty_points;
    Polynomial Z_empty = Polynomial::vanishing(empty_points);
    assert(Z_empty.deg() == 0);
    assert(Z_empty.coeff(0) == Fr(1));
    
    // Test single point vanishing polynomial
    std::vector<Fr> single_point = {Fr(5)};
    Polynomial Z_single = Polynomial::vanishing(single_point);
    assert(Z_single.deg() == 1);
    assert(Polynomial::eval(Z_single, Fr(5)) == Fr(0));
    
    std::cout << "Vanishing polynomial test passed!" << std::endl;
}

void test_shift_operations() {
    std::cout << "Testing shift operations..." << std::endl;
    
    // Test multiplying by x^k
    std::vector<Fr> coeffs = {Fr(1), Fr(2), Fr(3)}; // 1 + 2x + 3x^2
    Polynomial p(coeffs);
    
    // Multiply by x^2 should give x^2 + 2x^3 + 3x^4
    Polynomial shifted = Polynomial::mul_xk(p, 2);
    
    assert(shifted.coeff(0) == Fr(0));
    assert(shifted.coeff(1) == Fr(0));
    assert(shifted.coeff(2) == Fr(1));
    assert(shifted.coeff(3) == Fr(2));
    assert(shifted.coeff(4) == Fr(3));
    assert(shifted.deg() == 4);
    
    // Test shift by 0 (should be identity)
    Polynomial unshifted = Polynomial::mul_xk(p, 0);
    assert(unshifted.equals(p));
    
    // Test shift zero polynomial
    Polynomial zero_shifted = Polynomial::mul_xk(Polynomial::zero(), 5);
    assert(zero_shifted.is_zero());
    
    std::cout << "Shift operations test passed!" << std::endl;
}

void test_scalar_multiplication_advanced() {
    std::cout << "Testing advanced scalar multiplication..." << std::endl;
    
    // Test scalar multiplication
    std::vector<Fr> coeffs = {Fr(1), Fr(2), Fr(3)}; // 1 + 2x + 3x^2
    Polynomial p(coeffs);
    
    // Multiply by 5
    Polynomial scaled = Polynomial::scalar_mul(p, Fr(5));
    assert(scaled.coeff(0) == Fr(5));
    assert(scaled.coeff(1) == Fr(10));
    assert(scaled.coeff(2) == Fr(15));
    
    // Multiply by 0 should give zero polynomial
    Polynomial zero_scaled = Polynomial::scalar_mul(p, Fr(0));
    assert(zero_scaled.is_zero());
    
    // Test in-place scalar multiplication
    Polynomial p_copy = p;
    Polynomial::scalar_mul_inplace(p_copy, Fr(5));
    assert(p_copy.equals(scaled));
    
    // Test in-place with zero
    Polynomial::scalar_mul_inplace(p_copy, Fr(0));
    assert(p_copy.is_zero());
    
    std::cout << "Advanced scalar multiplication test passed!" << std::endl;
}

void test_edge_cases() {
    std::cout << "Testing edge cases..." << std::endl;
    
    // Test normalization with multiple trailing zeros
    std::vector<Fr> coeffs_zeros = {Fr(1), Fr(2), Fr(0), Fr(0), Fr(0)};
    Polynomial p_zeros(coeffs_zeros);
    assert(p_zeros.deg() == 1);
    assert(p_zeros.size() == 2);
    
    // Test arithmetic with zero polynomial
    Polynomial zero_p = Polynomial::zero();
    Polynomial one_p = Polynomial::one();
    
    assert(Polynomial::add(zero_p, one_p).equals(one_p));
    assert(Polynomial::sub(one_p, zero_p).equals(one_p));
    assert(Polynomial::mul_schoolbook(zero_p, one_p).is_zero());
    
    // Test set_coeff with auto-growing
    Polynomial p_grow = Polynomial::zero();
    p_grow.set_coeff(5, Fr(7));
    assert(p_grow.deg() == 5);
    assert(p_grow.coeff(5) == Fr(7));
    assert(p_grow.coeff(3) == Fr(0)); // Should be zero
    
    // Test access beyond bounds
    assert(p_grow.coeff(10) == Fr(0)); // Should return zero reference
    
    std::cout << "Edge cases test passed!" << std::endl;
}

void test_utility_methods() {
    std::cout << "Testing utility methods..." << std::endl;
    
    // Test string representation
    std::vector<Fr> coeffs = {Fr(2), Fr(3), Fr(1)}; // 2 + 3x + x^2
    Polynomial p(coeffs);
    std::string str = p.to_string();
    // Just check it doesn't crash and returns something
    assert(!str.empty());
    
    // Test equals method
    Polynomial p1({Fr(1), Fr(2), Fr(3)});
    Polynomial p2({Fr(1), Fr(2), Fr(3)});
    Polynomial p3({Fr(1), Fr(2), Fr(4)});
    
    assert(p1.equals(p2));
    assert(!p1.equals(p3));
    
    // Test with normalization
    Polynomial p4({Fr(1), Fr(2), Fr(3), Fr(0), Fr(0)});
    assert(p1.equals(p4)); // Should be equal after normalization
    
    std::cout << "Utility methods test passed!" << std::endl;
}

int main() {
    try {
        std::cout << "=== Polynomial Tests ===" << std::endl;
        
        test_polynomial_construction();
        test_polynomial_arithmetic();
        test_polynomial_evaluation();
        test_polynomial_interpolation();
        test_polynomial_division();
        test_vanishing_polynomial();
        test_shift_operations();
        test_scalar_multiplication_advanced();
        test_edge_cases();
        test_utility_methods();
        
        std::cout << "\nAll polynomial tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
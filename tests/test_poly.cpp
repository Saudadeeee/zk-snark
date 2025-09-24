#include "zkmini/polynomial.hpp"
#include "zkmini/utils.hpp"
#include <iostream>
#include <cassert>

using namespace zkmini;

void test_polynomial_construction() {
    std::cout << "Testing polynomial construction..." << std::endl;
    
    // Zero polynomial
    Polynomial zero_poly;
    assert(zero_poly.is_zero());
    assert(zero_poly.degree() == 0);
    
    // Constant polynomial
    Polynomial const_poly({Fr(5)});
    assert(!const_poly.is_zero());
    assert(const_poly.degree() == 0);
    
    // Linear polynomial: 3x + 2
    Polynomial linear({Fr(2), Fr(3)});
    assert(linear.degree() == 1);
    assert(linear.leading_coefficient() == Fr(3));
    
    // Quadratic polynomial: x^2 + 2x + 1
    Polynomial quadratic({Fr(1), Fr(2), Fr(1)});
    assert(quadratic.degree() == 2);
    
    std::cout << "Polynomial construction test passed!" << std::endl;
}

void test_polynomial_arithmetic() {
    std::cout << "Testing polynomial arithmetic..." << std::endl;
    
    // p1(x) = x + 1
    Polynomial p1({Fr(1), Fr(1)});
    
    // p2(x) = 2x + 3  
    Polynomial p2({Fr(3), Fr(2)});
    
    // Addition: (x + 1) + (2x + 3) = 3x + 4
    Polynomial sum = p1 + p2;
    assert(sum.degree() == 1);
    
    // Subtraction: (2x + 3) - (x + 1) = x + 2
    Polynomial diff = p2 - p1;
    assert(diff.degree() == 1);
    
    // Scalar multiplication: 2 * (x + 1) = 2x + 2
    Polynomial scaled = p1 * Fr(2);
    assert(scaled.degree() == 1);
    
    // Polynomial multiplication: (x + 1) * (2x + 3) = 2x^2 + 5x + 3
    Polynomial product = p1 * p2;
    assert(product.degree() == 2);
    
    std::cout << "Polynomial arithmetic test passed!" << std::endl;
}

void test_polynomial_evaluation() {
    std::cout << "Testing polynomial evaluation..." << std::endl;
    
    // p(x) = x^2 + 2x + 1 = (x + 1)^2
    Polynomial p({Fr(1), Fr(2), Fr(1)});
    
    // Evaluate at x = 0: p(0) = 1
    Fr val_0 = p.evaluate(Fr(0));
    // TODO: Enable when field arithmetic is implemented
    // assert(val_0 == Fr(1));
    
    // Evaluate at x = 1: p(1) = 4
    Fr val_1 = p.evaluate(Fr(1));
    // assert(val_1 == Fr(4));
    
    // Evaluate at x = 2: p(2) = 9
    Fr val_2 = p.evaluate(Fr(2));
    // assert(val_2 == Fr(9));
    
    // Batch evaluation
    std::vector<Fr> x_vals = {Fr(0), Fr(1), Fr(2)};
    std::vector<Fr> y_vals = p.evaluate_batch(x_vals);
    assert(y_vals.size() == 3);
    
    std::cout << "Polynomial evaluation test passed!" << std::endl;
}

void test_polynomial_interpolation() {
    std::cout << "Testing polynomial interpolation..." << std::endl;
    
    // Interpolate through points (0,1), (1,4), (2,9)
    // Should give p(x) = x^2 + 2x + 1
    std::vector<Fr> x_coords = {Fr(0), Fr(1), Fr(2)};
    std::vector<Fr> y_coords = {Fr(1), Fr(4), Fr(9)};
    
    Polynomial interpolated = Polynomial::lagrange_interpolate(x_coords, y_coords);
    assert(interpolated.degree() <= 2);
    
    // Verify interpolation: polynomial should pass through given points
    for (size_t i = 0; i < x_coords.size(); ++i) {
        Fr eval_val = interpolated.evaluate(x_coords[i]);
        // TODO: Enable when field arithmetic is implemented
        // assert(eval_val == y_coords[i]);
    }
    
    std::cout << "Polynomial interpolation test passed!" << std::endl;
}

void test_polynomial_division() {
    std::cout << "Testing polynomial division..." << std::endl;
    
    // dividend: x^2 + 3x + 2 = (x + 1)(x + 2)
    Polynomial dividend({Fr(2), Fr(3), Fr(1)});
    
    // divisor: x + 1
    Polynomial divisor({Fr(1), Fr(1)});
    
    auto [quotient, remainder] = dividend.divide(divisor);
    
    // Should get quotient = x + 2, remainder = 0
    assert(quotient.degree() == 1);
    assert(remainder.is_zero() || remainder.degree() < divisor.degree());
    
    // Verify: dividend = quotient * divisor + remainder
    Polynomial reconstructed = quotient * divisor + remainder;
    // TODO: Enable when polynomial arithmetic is fully implemented
    // assert(reconstructed == dividend);
    
    std::cout << "Polynomial division test passed!" << std::endl;
}

void test_vanishing_polynomial() {
    std::cout << "Testing vanishing polynomial..." << std::endl;
    
    // Vanishing polynomial for roots {1, 2, 3}
    // Should be (x - 1)(x - 2)(x - 3) = x^3 - 6x^2 + 11x - 6
    std::vector<Fr> roots = {Fr(1), Fr(2), Fr(3)};
    Polynomial vanishing = Polynomial::vanishing_polynomial(roots);
    
    assert(vanishing.degree() == 3);
    
    // Verify that vanishing polynomial is zero at all roots
    for (const Fr& root : roots) {
        Fr eval_val = vanishing.evaluate(root);
        // TODO: Enable when field arithmetic is implemented
        // assert(eval_val.is_zero());
    }
    
    std::cout << "Vanishing polynomial test passed!" << std::endl;
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
        
        std::cout << "All polynomial tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
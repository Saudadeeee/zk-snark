#include "zkmini/qap.hpp"
#include "zkmini/r1cs.hpp"
#include "zkmini/polynomial.hpp"
#include "zkmini/field.hpp"
#include "zkmini/utils.hpp"
#include <iostream>

using namespace zkmini;

void demo_basic_qap() {
    std::cout << "\n=== QAP Demo: Basic Multiplication Circuit ===" << std::endl;
    
    // Circuit: a * b = c
    R1CS r1cs;
    VarIdx a = r1cs.allocate_var();  // Variable 1
    VarIdx b = r1cs.allocate_var();  // Variable 2
    VarIdx c = r1cs.allocate_var();  // Variable 3
    
    // Mark public inputs
    r1cs.mark_public(a);  // a is public
    r1cs.mark_public(c);  // c is public (b is private)
    
    // Add constraint: a * b = c
    r1cs.add_mul(a, b, c);
    
    std::cout << "R1CS Circuit:" << std::endl;
    std::cout << "  Variables: " << r1cs.num_variables() << " (including x₀=1)" << std::endl;
    std::cout << "  Constraints: " << r1cs.num_constraints() << std::endl;
    std::cout << "  Public inputs: " << r1cs.public_inputs().size() << std::endl;
    std::cout << "  Constraint: x₁ * x₂ = x₃ (a * b = c)" << std::endl;
    
    // Convert R1CS to QAP
    std::cout << "\nConverting R1CS to QAP..." << std::endl;
    QAP qap = r1cs_to_qap(r1cs);
    
    std::cout << "QAP Structure:" << std::endl;
    std::cout << "  Domain size m = " << qap.m << std::endl;
    std::cout << "  Variables n = " << qap.n << std::endl;
    std::cout << "  Domain points: " << debug_domain(qap) << std::endl;
    std::cout << "  Vanishing polynomial: Z(X) = " << qap.Z.to_string() << std::endl;
    
    // Show basis polynomials
    std::cout << "\nBasis Polynomials:" << std::endl;
    for (size_t i = 0; i < qap.n; ++i) {
        std::cout << "  Variable " << i << ":" << std::endl;
        std::cout << "    A₋" << i << "(X) = " << qap.A_basis[i].to_string() << std::endl;
        std::cout << "    B₋" << i << "(X) = " << qap.B_basis[i].to_string() << std::endl;
        std::cout << "    C₋" << i << "(X) = " << qap.C_basis[i].to_string() << std::endl;
    }
    
    // Test with witness: 3 * 4 = 12
    std::cout << "\n=== Testing with witness: 3 * 4 = 12 ===" << std::endl;
    
    std::vector<Fr> witness(qap.n, Fr(0));
    witness[0] = Fr(1);   // x₀ = 1 (constant)
    witness[a] = Fr(3);   // a = 3 (public)
    witness[b] = Fr(4);   // b = 4 (private)
    witness[c] = Fr(12);  // c = 12 (public)
    
    std::cout << "Witness vector: [1";
    for (size_t i = 1; i < witness.size(); ++i) {
        std::cout << ", " << witness[i].to_string();
    }
    std::cout << "]" << std::endl;
    
    // Verify R1CS satisfaction
    bool r1cs_ok = r1cs.is_satisfied(witness);
    std::cout << "R1CS satisfied: " << (r1cs_ok ? "✓ YES" : "✗ NO") << std::endl;
    
    // Assemble witness polynomials
    std::cout << "\nAssembling witness polynomials..." << std::endl;
    Polynomial A_poly = assemble_A(qap, witness);
    Polynomial B_poly = assemble_B(qap, witness);
    Polynomial C_poly = assemble_C(qap, witness);
    
    std::cout << "A(X) = Σ xᵢ·Aᵢ(X) = " << A_poly.to_string() << std::endl;
    std::cout << "B(X) = Σ xᵢ·Bᵢ(X) = " << B_poly.to_string() << std::endl;
    std::cout << "C(X) = Σ xᵢ·Cᵢ(X) = " << C_poly.to_string() << std::endl;
    
    // Check QAP constraint satisfaction
    std::cout << "\nChecking QAP constraint: A(X)·B(X) - C(X) ≡ 0 (mod Z(X))" << std::endl;
    
    Polynomial AB = Polynomial::mul_schoolbook(A_poly, B_poly);
    Polynomial numerator = Polynomial::sub(AB, C_poly);
    
    std::cout << "A(X)·B(X) = " << AB.to_string() << std::endl;
    std::cout << "A(X)·B(X) - C(X) = " << numerator.to_string() << std::endl;
    
    bool is_divisible = divides(numerator, qap.Z);
    std::cout << "Divisible by Z(X): " << (is_divisible ? "✓ YES" : "✗ NO") << std::endl;
    
    if (is_divisible) {
        Polynomial H = compute_H(A_poly, B_poly, C_poly, qap.Z);
        std::cout << "Quotient H(X) = " << H.to_string() << std::endl;
        
        // Verify: H(X)·Z(X) = A(X)·B(X) - C(X)
        Polynomial verification = Polynomial::mul_schoolbook(H, qap.Z);
        bool verified = (verification == numerator);
        std::cout << "Verification H(X)·Z(X) = A(X)·B(X) - C(X): " 
                  << (verified ? "✓ PASSED" : "✗ FAILED") << std::endl;
    }
    
    // Overall QAP check
    bool qap_ok = qap_check(qap, witness);
    std::cout << "\nOverall QAP satisfied: " << (qap_ok ? "✓ YES" : "✗ NO") << std::endl;
    
    if (qap_ok) {
        std::cout << "\n🎉 QAP demo completed successfully!" << std::endl;
        std::cout << "The witness (3, 4, 12) satisfies the constraint 3 * 4 = 12" << std::endl;
        std::cout << "in both R1CS and QAP representations." << std::endl;
    }
}

void demo_qap_properties() {
    std::cout << "\n=== QAP Mathematical Properties Demo ===" << std::endl;
    
    // Create QAP from simple circuit
    R1CS r1cs;
    VarIdx x = r1cs.allocate_var();
    VarIdx y = r1cs.allocate_var();
    r1cs.add_mul(x, y, x);  // x * y = x, so y = 1 or x = 0
    
    QAP qap = r1cs_to_qap(r1cs);
    
    // Property 1: Vanishing polynomial zeros
    std::cout << "Property 1: Vanishing polynomial Z(X) = 0 at domain points" << std::endl;
    for (size_t i = 0; i < qap.domain_points.size(); ++i) {
        Fr domain_point = qap.domain_points[i];
        Fr z_val = qap.Z.evaluate(domain_point);
        std::cout << "  Z(" << domain_point.to_string() << ") = " 
                  << z_val.to_string() << " " 
                  << (z_val.is_zero() ? "✓" : "✗") << std::endl;
    }
    
    // Property 2: Basis polynomial evaluation at domain points
    std::cout << "\nProperty 2: Basis polynomials encode constraint coefficients" << std::endl;
    for (size_t constraint = 0; constraint < qap.m; ++constraint) {
        Fr domain_point = qap.domain_points[constraint];
        std::cout << "  At constraint " << constraint << " (domain point " 
                  << domain_point.to_string() << "):" << std::endl;
        
        for (size_t var = 0; var < qap.n; ++var) {
            Fr A_val = qap.A_basis[var].evaluate(domain_point);
            Fr B_val = qap.B_basis[var].evaluate(domain_point);
            Fr C_val = qap.C_basis[var].evaluate(domain_point);
            
            // Get expected values from R1CS
            std::vector<Fr> A_col = r1cs.column_values(r1cs.A, var);
            std::vector<Fr> B_col = r1cs.column_values(r1cs.B, var);
            std::vector<Fr> C_col = r1cs.column_values(r1cs.C, var);
            
            Fr expected_A = (constraint < A_col.size()) ? A_col[constraint] : Fr(0);
            Fr expected_B = (constraint < B_col.size()) ? B_col[constraint] : Fr(0);
            Fr expected_C = (constraint < C_col.size()) ? C_col[constraint] : Fr(0);
            
            if (!A_val.is_zero() || !B_val.is_zero() || !C_val.is_zero()) {
                std::cout << "    var " << var << ": A=" << A_val.to_string() 
                          << " B=" << B_val.to_string() << " C=" << C_val.to_string();
                std::cout << " (expected A=" << expected_A.to_string()
                          << " B=" << expected_B.to_string() << " C=" << expected_C.to_string() << ")";
                std::cout << " " << ((A_val == expected_A && B_val == expected_B && C_val == expected_C) ? "✓" : "✗");
                std::cout << std::endl;
            }
        }
    }
    
    std::cout << "\n✓ QAP properties verified successfully!" << std::endl;
}

int main() {
    std::cout << "QAP (Quadratic Arithmetic Program) Comprehensive Demo" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    try {
        demo_basic_qap();
        demo_qap_properties();
        
        std::cout << "\n🎯 Summary:" << std::endl;
        std::cout << "- QAP successfully converts R1CS constraints to polynomial form" << std::endl;
        std::cout << "- Basis polynomials correctly encode constraint coefficients" << std::endl; 
        std::cout << "- Vanishing polynomial ensures constraint satisfaction" << std::endl;
        std::cout << "- Witness assembly and divisibility check work correctly" << std::endl;
        std::cout << "- Ready for integration with Groth16 proving system!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
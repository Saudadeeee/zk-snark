#!/bin/bash

# ZK-SNARK Complete Test Suite
echo "============================================================"
echo "  ZK-SNARK Complete Test Suite"
echo "============================================================"
echo ""

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_executable="$2"
    local description="$3"
    
    echo "--- Testing: $description ---"
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ -f "$test_executable" ]; then
        if timeout 30s ./"$test_executable" > /dev/null 2>&1; then
            echo "✓ PASS: $test_name"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo "✗ FAIL: $test_name"
            FAILED_TESTS=$((FAILED_TESTS + 1))
            echo "  Running with output for debugging:"
            ./"$test_executable" 2>&1 | head -20
        fi
    else
        echo "✗ SKIP: $test_name (executable not found)"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    echo ""
}

# Navigate to build directory
cd "$(dirname "$0")/build" || exit 1

echo "🧪 Running Core Component Tests..."
echo ""

# Core mathematical tests
run_test "Field_BN254" "test_field_bn254" "BN254 Field Arithmetic"
run_test "Field_Comprehensive" "test_field_comprehensive" "Comprehensive Field Properties"
run_test "Elliptic_Curves" "test_ec" "Elliptic Curve Operations"
run_test "Pairing" "test_pairing" "Bilinear Pairing Operations"
run_test "Polynomials" "test_poly" "Polynomial Arithmetic"

echo "🔗 Running ZK-SNARK Protocol Tests..."
echo ""

# Protocol tests
run_test "R1CS_New" "test_r1cs_new" "R1CS Constraint System"
run_test "R1CS_Comprehensive" "test_r1cs_comprehensive" "R1CS Advanced Features"
run_test "QAP_Simple" "test_qap_simple" "QAP Basic Conversion"
run_test "QAP_Full" "test_qap" "QAP Complete Implementation"
run_test "Groth16" "test_groth16" "Groth16 Proving System"

echo "🎯 Running Integration Tests..."
echo ""

# Demo and integration tests
run_test "QAP_Demo" "qap_demo" "QAP Educational Demo"

echo "============================================================"
echo "  Test Results Summary"
echo "============================================================"
echo ""
echo "Total Tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "🎉 ALL TESTS PASSED!"
    echo "✅ ZK-SNARK library is working correctly"
    echo ""
    echo "🚀 Ready for:"
    echo "   - Academic research"
    echo "   - Educational use"
    echo "   - Prototype development"
    echo "   - Production integration"
else
    echo "⚠️  Some tests failed ($FAILED_TESTS/$TOTAL_TESTS)"
    echo "📝 Check individual test outputs above for details"
    echo ""
    echo "💡 Common issues:"
    echo "   - Field arithmetic edge cases"
    echo "   - Elliptic curve point validation"
    echo "   - Pairing implementation details"
fi

echo ""
echo "============================================================"

# Return appropriate exit code
if [ $FAILED_TESTS -eq 0 ]; then
    exit 0
else
    exit 1
fi
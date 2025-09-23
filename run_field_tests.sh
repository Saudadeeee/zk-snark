#!/bin/bash

# Field Test Runner Script
# Chạy tất cả các test field và tóm tắt kết quả

echo "=========================================="
echo "🧪 ZK-SNARK FIELD COMPREHENSIVE TEST SUITE"
echo "=========================================="
echo ""

# Build directory path
BUILD_DIR="build"
TESTS_PASSED=0
TESTS_TOTAL=0

# Function to run test and capture result
run_test() {
    local test_name="$1"
    local test_path="$BUILD_DIR/$test_name"
    
    echo "🔄 Running $test_name..."
    
    if [ -f "$test_path" ]; then
        if ./"$test_path" > /dev/null 2>&1; then
            echo "✅ $test_name: PASSED"
            ((TESTS_PASSED++))
        else
            echo "❌ $test_name: FAILED"
            echo "   Running again with output:"
            ./"$test_path"
        fi
    else
        echo "⚠️  $test_name: NOT FOUND (skipped)"
    fi
    
    ((TESTS_TOTAL++))
    echo ""
}

# Run individual field tests
echo "Running individual field tests..."
echo "─────────────────────────────────────────"
run_test "test_field"
run_test "test_field_basic"
run_test "test_field_comprehensive"

# Summary
echo "=========================================="
echo "📊 TEST SUMMARY"
echo "=========================================="
echo "Tests passed: $TESTS_PASSED/$TESTS_TOTAL"

if [ $TESTS_PASSED -eq $TESTS_TOTAL ]; then
    echo "🎉 ALL FIELD TESTS PASSED!"
    echo ""
    echo "✅ Field implementation verification complete:"
    echo "   • Additive group properties verified"
    echo "   • Multiplicative properties verified"
    echo "   • Distributive law verified"
    echo "   • Division/inverse operations verified"
    echo "   • Fermat's Little Theorem verified"
    echo "   • Boundary cases handled correctly"
    echo "   • Serialization round-trip works"
    echo "   • Performance benchmarks completed"
    echo ""
    echo "🚀 Ready for next phase: Elliptic Curves & Polynomial arithmetic"
    exit 0
else
    echo "❌ SOME TESTS FAILED!"
    echo "Please fix the failing tests before proceeding."
    exit 1
fi
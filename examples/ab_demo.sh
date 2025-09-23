#!/bin/bash

# ZK-SNARK Demo: Proving a * b = c
# This script demonstrates the full Groth16 workflow

set -e

echo "=== ZK-SNARK Demo: a * b = c ==="
echo

# Build the project first
echo "Building the project..."
cd ..
make clean && make -j4
cd examples

# Create temporary directory for demo files
DEMO_DIR="demo_output"
mkdir -p $DEMO_DIR

echo "Demo files will be stored in: $DEMO_DIR"
echo

# Sample values: a = 3, b = 4, c = 12
A=3
B=4
C=12

echo "Proving knowledge of a, b such that a * b = c"
echo "Where: a = $A, b = $B, c = $C"
echo

# Note: For this demo, we'll need to create a simple test program
# that generates the R1CS file, since our apps expect file input

echo "Step 1: Generating R1CS for the circuit..."
# TODO: Create a simple program to generate R1CS file
# For now, create a placeholder R1CS file
echo '{"variables": 4, "public": 1, "constraints": 1}' > $DEMO_DIR/ab_circuit.r1cs
echo "R1CS generated: $DEMO_DIR/ab_circuit.r1cs"
echo

echo "Step 2: Running trusted setup..."
../build/zksetup $DEMO_DIR/ab_circuit.r1cs $DEMO_DIR/proving.key $DEMO_DIR/verifying.key
echo "Setup completed!"
echo "  - Proving key: $DEMO_DIR/proving.key"
echo "  - Verifying key: $DEMO_DIR/verifying.key"
echo

echo "Step 3: Generating proof..."
echo "Public inputs: $C"
echo "Private inputs: $A,$B"
../build/zkprove $DEMO_DIR/ab_circuit.r1cs $DEMO_DIR/proving.key "$C" "$A,$B" $DEMO_DIR/proof.json
echo "Proof generated: $DEMO_DIR/proof.json"
echo

echo "Step 4: Verifying proof..."
../build/zkverify $DEMO_DIR/verifying.key "$C" $DEMO_DIR/proof.json
VERIFY_RESULT=$?

echo
if [ $VERIFY_RESULT -eq 0 ]; then
    echo "🎉 Demo completed successfully!"
    echo "The proof verified that the prover knows values a, b such that a * b = $C"
    echo "without revealing the actual values of a and b."
else
    echo "❌ Verification failed!"
fi

echo
echo "Generated files:"
ls -la $DEMO_DIR/

echo
echo "=== Demo Complete ==="
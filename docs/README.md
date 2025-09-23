# ZKMini Documentation

## Overview

ZKMini is a C++ implementation of the Groth16 zero-knowledge proof system using the BN254 elliptic curve.

## Architecture

### Core Components

1. **Field Arithmetic (`field.hpp`)**
   - Scalar field Fr (BN254 base field)
   - Extension fields Fq, Fq2, Fq6, Fq12

2. **Elliptic Curves (`ec.hpp`)**
   - G1 group (E(Fq): y² = x³ + 3)
   - G2 group (E(Fq²): y² = x³ + 3/(9+u))

3. **Bilinear Pairing (`pairing.hpp`)**
   - Optimal Ate pairing on BN254
   - Miller loop and final exponentiation

4. **Polynomial Operations (`polynomial.hpp`)**
   - Univariate polynomials over Fr
   - Fast Fourier Transform (FFT)
   - Polynomial arithmetic

5. **Constraint Systems (`r1cs.hpp`, `qap.hpp`)**
   - R1CS (Rank-1 Constraint System)
   - QAP (Quadratic Arithmetic Program)
   - Lagrange interpolation

6. **Groth16 Protocol (`groth16.hpp`)**
   - Trusted setup
   - Proof generation
   - Verification

## Mathematical Background

### BN254 Curve Parameters

The BN254 elliptic curve is defined over two fields:

- **Base field Fq**: q = 21888242871839275222246405745257275088696311157297823662689037894645226208583
- **Scalar field Fr**: r = 21888242871839275222246405745257275088548364400416034343698204186575808495617

### Groth16 Protocol

Groth16 is a preprocessing zk-SNARK that allows proving knowledge of a satisfying assignment to a QAP.

**Setup Phase:**
1. Convert circuit to R1CS
2. Transform R1CS to QAP  
3. Generate proving and verifying keys

**Prove Phase:**
1. Compute witness values
2. Evaluate QAP polynomials
3. Generate proof elements (A, B, C)

**Verify Phase:**
1. Check pairing equation: e(A,B) = e(α,β) · e(L,γ) · e(C,δ)

## Usage Examples

See `/examples` directory for complete circuit examples:

- `ab_circuit.cpp` - Simple multiplication circuit
- `quadratic_circuit.cpp` - Polynomial evaluation

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Testing

```bash
cd build
make test
```

## Security Considerations

⚠️ **WARNING**: This is a reference implementation for educational purposes. It has not been audited and should not be used in production systems.

### Known Limitations

1. No constant-time implementations
2. Basic random number generation
3. Simplified field arithmetic
4. No side-channel protections

## Contributing

1. Implement cryptographic operations marked with TODO
2. Add comprehensive tests
3. Optimize performance-critical sections
4. Add security hardening

## References

- [Groth16 Paper](https://eprint.iacr.org/2016/260.pdf)
- [BN254 Specification](https://eprint.iacr.org/2005/133.pdf)
- [libsnark](https://github.com/scipr-lab/libsnark)
- [bellman](https://github.com/zkcrypto/bellman)
# ZKMini Design Document

## Overview

ZKMini là một thư viện C++ implementation của hệ thống zero-knowledge proof Groth16, cho phép chứng minh kiến thức về một witness bí mật thỏa mãn một arithmetic circuit mà không cần tiết lộ witness đó.

### Zero-Knowledge Proof là gì?

Zero-Knowledge Proof (ZKP) là một phương pháp mật mã học cho phép một bên (prover) chứng minh với bên kia (verifier) rằng họ biết một thông tin bí mật mà không cần tiết lộ thông tin đó.

**Ví dụ thực tế**: 
- Alice muốn chứng minh với Bob rằng cô ấy biết mật khẩu của một tài khoản mà không cần nói mật khẩu đó là gì
- Một công ty muốn chứng minh họ tuân thủ quy định về thuế mà không cần tiết lộ doanh thu thực tế

### Groth16 Protocol

Groth16 là một trong những ZK-SNARK (Zero-Knowledge Succinct Non-Interactive Arguments of Knowledge) hiệu quả nhất hiện tại:

**Đặc điểm chính**:
- **Succinct**: Proof size cố định (chỉ 3 group elements ≈ 256 bytes)
- **Non-Interactive**: Không cần tương tác giữa prover và verifier
- **Fast Verification**: Thời gian verify rất nhanh (≈3ms)
- **Preprocessing**: Cần trusted setup một lần cho mỗi circuit

## System Architecture

ZKMini được thiết kế theo kiến trúc nhiều lớp (layered architecture) để đảm bảo tính modular và dễ bảo trì:

```
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │   Applications  │    │    Examples     │    │     Tests       │
    │  zksetup/prove/ │    │   ab_circuit/   │    │  Unit/Bench/    │
    │     verify      │    │  quadratic      │    │  Integration    │
    └─────────────────┘    └─────────────────┘    └─────────────────┘
                ▲                    ▲                       ▲
                │                    │                       │
    ════════════════════════════════════════════════════════════════
                │              ZKMini Library                │
    ════════════════════════════════════════════════════════════════
                ▼                    ▼                       ▼
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │  Groth16 Layer  │    │ Constraint Sys  │    │  Utility Layer  │
    │   setup/prove/  │◄──►│   R1CS/QAP/     │◄──►│   Utils/Random/ │
    │     verify      │    │   Lagrange      │    │  Serialization  │
    └─────────────────┘    └─────────────────┘    └─────────────────┘
                ▲                    ▲                       ▲
                │                    │                       │
    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
    │ Polynomial Layer│    │  Pairing Layer  │    │   Field Layer   │
    │    FFT/Poly     │◄──►│ Optimal Ate/    │◄──►│   Fr/Fq/Fq2/   │
    │   Arithmetic    │    │  Miller Loop    │    │   Fq6/Fq12      │
    └─────────────────┘    └─────────────────┘    └─────────────────┘
                                     ▲                       ▲
                                     │                       │
                           ┌─────────────────┐    ┌─────────────────┐
                           │ Elliptic Curve  │    │ Montgomery Arith│
                           │    G1/G2/MSM    │◄──►│  ModMul/ModInv/ │
                           │   Point Ops     │    │    ModExp       │
                           └─────────────────┘    └─────────────────┘
```

### Giải thích các lớp:

1. **Application Layer**: Các công cụ command-line cho người dùng cuối
2. **Groth16 Layer**: Implementation của protocol Groth16
3. **Constraint System Layer**: Chuyển đổi circuit thành constraint system
4. **Polynomial Layer**: Xử lý các phép toán đa thức
5. **Pairing Layer**: Thực hiện bilinear pairing operations
6. **Field Layer**: Số học trên finite fields
7. **Elliptic Curve Layer**: Phép toán trên elliptic curves
8. **Montgomery Arithmetic**: Số học modular hiệu quả\n\n## Core Components\n\n### 1. Field Arithmetic Layer\n\n**Purpose**: Efficient arithmetic in finite fields\n\n**Components**:\n- `Fr`: BN254 scalar field (256-bit prime)\n- `Fq`: BN254 base field (256-bit prime) \n- `Fq2, Fq6, Fq12`: Tower field extensions for pairing\n\n**Key Operations**:\n- Montgomery multiplication for efficiency\n- Modular inversion using extended Euclidean algorithm\n- Fast modular exponentiation\n\n**Implementation Notes**:\n```cpp\nclass Fr {\n    uint64_t data[4];  // 256-bit representation\n    \n    // Montgomery constants\n    static const Fr R;    // 2^256 mod p\n    static const Fr R2;   // 2^512 mod p\n    static const uint64_t INV; // -p^(-1) mod 2^64\n};\n```\n\n### 2. Elliptic Curve Layer\n\n**Purpose**: Group operations on BN254 curve\n\n**Components**:\n- `G1`: Points on E(Fq): y² = x³ + 3\n- `G2`: Points on E(Fq²): y² = x³ + 3/(9+u)\n\n**Key Operations**:\n- Point addition/doubling in Jacobian coordinates\n- Scalar multiplication using windowed NAF\n- Multi-scalar multiplication (MSM) using Pippenger's algorithm\n\n**Implementation Notes**:\n```cpp\nclass G1 {\n    Fq x, y, z;  // Jacobian coordinates (x/z², y/z³)\n    \n    static G1 double_impl(const G1& p);\n    static G1 add_impl(const G1& p, const G1& q);\n};\n```\n\n### 3. Pairing Layer\n\n**Purpose**: Bilinear map e: G1 × G2 → GT\n\n**Components**:\n- Miller loop computation\n- Final exponentiation\n- Pairing optimizations (denominator elimination)\n\n**Algorithm**: Optimal Ate pairing on BN254\n```\ne(P, Q) = f_{6t+2}(P, Q)^{(p^12-1)/r}\n```\n\n**Implementation Strategy**:\n1. Line evaluation at each doubling/addition step\n2. Sparse multiplication in Fq12\n3. Fast final exponentiation using cyclotomic subgroup\n\n### 4. Polynomial Layer\n\n**Purpose**: Polynomial operations over Fr\n\n**Components**:\n- Dense polynomial representation\n- Fast Fourier Transform (FFT)\n- Lagrange interpolation\n\n**Key Operations**:\n```cpp\nclass Polynomial {\n    std::vector<Fr> coeffs;\n    \n    Polynomial operator*(const Polynomial& other) const;\n    Fr evaluate_at(const Fr& x) const;\n    static Polynomial interpolate(const std::vector<Fr>& points,\n                                 const std::vector<Fr>& values);\n};\n```\n\n### 5. Constraint System Layer\n\n**Purpose**: Circuit representation and transformation\n\n**R1CS Format**:\n```\nFor each constraint i:\n(∑ aᵢⱼ · xⱼ) · (∑ bᵢⱼ · xⱼ) = (∑ cᵢⱼ · xⱼ)\n```\n\n**QAP Transformation**:\n1. Interpolate constraint matrices to polynomials\n2. Define target polynomial t(x) = ∏(x - rᵢ)\n3. Satisfying assignment produces h(x) such that:\n   A(x)·B(x) - C(x) = h(x)·t(x)\n\n### 6. Groth16 Protocol Layer\n\n**Setup Phase**:\n```\nInput: QAP (A, B, C, t)\nOutput: (proving_key, verifying_key)\n\n1. Sample random α, β, γ, δ ∈ Fr\n2. Compute proving key elements in G1, G2\n3. Compute verifying key for public inputs\n```\n\n**Prove Phase**:\n```\nInput: proving_key, witness (public + private)\nOutput: proof (A, B, C) ∈ G1 × G2 × G1\n\n1. Compute A(τ), B(τ), C(τ) from witness\n2. Sample random r, s ∈ Fr\n3. Construct proof elements with randomness\n```\n\n**Verify Phase**:\n```\nInput: verifying_key, public_inputs, proof\nOutput: accept/reject\n\nCheck: e(A, B) = e(α, β) · e(∑ aᵢ·γᵢ, γ) · e(C, δ)\n```\n\n## Security Properties\n\n### Completeness\nAny valid proof generated by an honest prover will be accepted by the verifier.\n\n### Soundness\nNo adversary can generate an accepting proof for a false statement (under cryptographic assumptions).\n\n### Zero-Knowledge\nProofs reveal no information about the witness beyond its validity.\n\n### Cryptographic Assumptions\n1. **Generic Group Model**: Adversary can only perform group operations\n2. **Knowledge of Exponent**: If adversary outputs (g^a, g^{ab}), it \"knows\" b\n3. **BN254 Security**: Discrete log hard in G1, G2, GT\n\n## Performance Considerations\n\n### Bottlenecks\n1. **Setup**: Expensive due to G1/G2 exponentiations\n2. **Prove**: MSM operations dominate\n3. **Verify**: Pairing computations are costly\n\n### Optimizations\n1. **Precomputation**: Cache powers for MSM\n2. **Parallelization**: Embarrassingly parallel MSM\n3. **Special Form**: Leverage BN254 curve properties\n\n### Complexity Analysis\n- **Setup**: O(|Circuit| · log |Circuit|)\n- **Prove**: O(|Circuit| · log |Circuit|)\n- **Verify**: O(|Public Inputs|)\n- **Proof Size**: O(1) - always 3 group elements\n\n## Implementation Phases\n\n### Phase 1: Foundation (Current)\n- Basic field arithmetic\n- Elliptic curve operations\n- Polynomial framework\n- Testing infrastructure\n\n### Phase 2: Cryptographic Core\n- Pairing implementation\n- FFT and polynomial ops\n- R1CS to QAP conversion\n\n### Phase 3: Protocol Implementation\n- Groth16 setup/prove/verify\n- Serialization/deserialization\n- Performance optimizations\n\n### Phase 4: Production Readiness\n- Constant-time implementations\n- Side-channel protections\n- Formal verification\n- Security audit\n\n## References\n\n1. Groth, J. \"On the Size of Pairing-based Non-interactive Arguments.\" EUROCRYPT 2016.\n2. Barreto, P., Naehrig, M. \"Pairing-Friendly Elliptic Curves of Prime Order.\" SAC 2005.\n3. Gabizon, A., et al. \"PLONK: Permutations over Lagrange-bases for Oecumenical Noninteractive arguments of Knowledge.\" IACR ePrint 2019.\n4. Ben-Sasson, E., et al. \"Scalable, transparent, and post-quantum secure computational integrity.\" IACR ePrint 2018.

---

## Supplementary Documentation

📖 **For detailed Vietnamese explanations with code examples**: See [`design_vietnamese_supplement.md`](./design_vietnamese_supplement.md)

This supplementary document includes:
- Comprehensive Vietnamese explanations of all concepts
- Step-by-step Groth16 protocol walkthrough with code
- Detailed mathematical background
- Real-world usage examples (age verification, voting, etc.)
- Performance analysis with specific numbers
- Security considerations and attack vectors
- Implementation details for key algorithms

The supplement provides practical understanding for Vietnamese developers working with ZK-SNARKs.

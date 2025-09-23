# ZKMini Design Document - Vietnamese Supplement

## Detailed Vietnamese Explanations (Giải thích chi tiết bằng tiếng Việt)

### Tổng quan về Zero-Knowledge Proofs

**Zero-Knowledge Proof là gì?**
Zero-Knowledge Proof (Chứng minh không kiến thức) là một phương pháp cho phép Alice chứng minh với Bob rằng cô ấy biết một thông tin bí mật mà không cần tiết lộ thông tin đó.

**3 tính chất quan trọng:**
1. **Completeness (Tính đầy đủ)**: Nếu statement đúng và Alice biết witness, Bob sẽ luôn accept proof
2. **Soundness (Tính đúng đắn)**: Nếu statement sai, Alice không thể tạo proof khiến Bob accept (với xác suất cao)
3. **Zero-Knowledge (Không kiến thức)**: Bob không học được gì về witness ngoài việc statement đúng

**Ví dụ thực tế:**
```
Alice muốn chứng minh: "Tôi biết x sao cho x² + 3x + 2 = 0"
- Mà không tiết lộ x = -1 hoặc x = -2
- Bob sẽ tin rằng Alice thật sự biết một nghiệm
- Nhưng Bob không biết nghiệm cụ thể là gì
```

### Groth16 Protocol - Giải thích từng bước

**Bước 1: Circuit Design**
```cpp
// Ví dụ: Circuit chứng minh biết password
// Input: password_hash (public), password (private)
// Constraint: SHA256(password) = password_hash

class PasswordCircuit {
public:
    // Variables: [1, password_hash, password, intermediate_values...]
    static R1CS create_r1cs() {
        R1CS r1cs;
        r1cs.num_variables = 1000;  // Tùy thuộc vào complexity của SHA256
        r1cs.num_public = 1;        // Chỉ password_hash là public
        r1cs.num_private = 999;     // password và intermediate values
        
        // Thêm constraints cho SHA256 computation
        // Mỗi step của SHA256 được biểu diễn bằng R1CS constraints
        return r1cs;
    }
    
    static std::pair<std::vector<Fr>, std::vector<Fr>> 
    create_witness(const std::string& password) {
        std::vector<Fr> public_inputs = {hash_password(password)};
        std::vector<Fr> private_inputs = {string_to_fr(password), /* intermediate */};
        return {public_inputs, private_inputs};
    }
};
```

**Bước 2: Trusted Setup**
```cpp
// Setup phase - chạy một lần cho mỗi circuit
CRS setup_ceremony() {
    // 1. Sample random values (phải được destroy sau setup!)
    Fr alpha = Fr::random();
    Fr beta = Fr::random(); 
    Fr gamma = Fr::random();
    Fr delta = Fr::random();
    Fr tau = Fr::random();  // "toxic waste" - phải bị xóa!
    
    // 2. Compute proving key elements
    ProvingKey pk;
    pk.alpha_g1 = alpha * G1::generator();
    pk.beta_g1 = beta * G1::generator();
    pk.delta_g1 = delta * G1::generator();
    
    // Compute A, B polynomials tại tau
    for (size_t i = 0; i < qap.num_variables; i++) {
        pk.A_g1[i] = qap.A[i].evaluate_at(tau) * G1::generator();
        pk.B_g1[i] = qap.B[i].evaluate_at(tau) * G1::generator();
        pk.B_g2[i] = qap.B[i].evaluate_at(tau) * G2::generator();
    }
    
    // 3. Compute verifying key
    VerifyingKey vk;
    vk.alpha_g1 = pk.alpha_g1;
    vk.beta_g2 = beta * G2::generator();
    vk.gamma_g2 = gamma * G2::generator();
    vk.delta_g2 = delta * G2::generator();
    
    // ⚠️ CRITICAL: Destroy toxic waste!
    // alpha.clear(); beta.clear(); gamma.clear(); delta.clear(); tau.clear();
    
    return {pk, vk};
}
```

**Bước 3: Proof Generation**
```cpp
Proof prove(const ProvingKey& pk, const QAP& qap,
            const std::vector<Fr>& public_inputs,
            const std::vector<Fr>& private_inputs) {
    
    // 1. Tạo full assignment
    auto assignment = qap.generate_full_assignment(public_inputs, private_inputs);
    
    // 2. Compute A, B, C values từ QAP
    Fr A_val = Fr::zero();
    Fr B_val = Fr::zero(); 
    Fr C_val = Fr::zero();
    
    for (size_t i = 0; i < assignment.size(); i++) {
        A_val += assignment[i] * qap.A[i].evaluate_at(tau);
        B_val += assignment[i] * qap.B[i].evaluate_at(tau);
        C_val += assignment[i] * qap.C[i].evaluate_at(tau);
    }
    
    // 3. Sample random values cho zero-knowledge
    Fr r = Fr::random();
    Fr s = Fr::random();
    
    // 4. Construct proof elements
    G1 proof_A = A_val * G1::generator() + r * pk.delta_g1;
    G2 proof_B = B_val * G2::generator() + s * pk.delta_g2;
    
    // Compute H polynomial coefficient
    Fr H_val = compute_h_polynomial_value(assignment, qap);
    G1 proof_C = H_val * pk.delta_g1 + s * proof_A;
    
    return Proof(proof_A, proof_B, proof_C);
}
```

**Bước 4: Verification**
```cpp
bool verify(const VerifyingKey& vk, 
           const std::vector<Fr>& public_inputs,
           const Proof& proof) {
    
    // 1. Compute public input contribution
    G1 vk_x = Fr::zero() * G1::generator();  // Start with zero
    for (size_t i = 0; i < public_inputs.size(); i++) {
        vk_x += public_inputs[i] * vk.gamma_abc_g1[i + 1];
    }
    vk_x += vk.gamma_abc_g1[0];  // Add constant term
    
    // 2. Check pairing equation:
    // e(A, B) = e(α, β) · e(vk_x, γ) · e(C, δ)
    
    Fq12 left = pairing(proof.A, proof.B);
    
    Fq12 right = pairing(vk.alpha_g1, vk.beta_g2) *
                 pairing(vk_x, vk.gamma_g2) *
                 pairing(proof.C, vk.delta_g2);
    
    return left == right;
}
```

### Tại sao Groth16 hoạt động?

**Intuition đằng sau pairing equation:**

1. **Setup tạo "trapdoor"**: Các giá trị α, β, γ, δ, τ là random và được giấu trong group elements
2. **Prover phải "biết" witness**: Để tạo proof hợp lệ, prover phải thật sự biết assignment thỏa mãn circuit
3. **Pairing check đảm bảo consistency**: Equation chỉ hold nếu proof được tạo correctly từ valid witness
4. **Randomness đảm bảo zero-knowledge**: r, s che giấu actual values trong proof

**Tại sao không thể fake proof?**
- Để fake proof, attacker cần biết discrete log của group elements
- Điều này tương đương với việc "biết" τ, α, β, γ, δ từ setup
- Nếu setup được thực hiện đúng và toxic waste bị destroy, điều này computationally impossible

### BN254 Curve - Tại sao chọn curve này?

**Đặc điểm đặc biệt của BN254:**
```
// Base field Fq (cho G1)
q = 21888242871839275222246405745257275088696311157297823662689037894645226208583

// Scalar field Fr (cho scalars)  
r = 21888242871839275222246405745257275088548364400416034343698204186575808495617

// Curve equation
G1: y² = x³ + 3 (over Fq)
G2: y² = x³ + 3/(9+u) (over Fq²)

// Embedding degree k = 12
// Có nghĩa là pairing map: G1 × G2 → Fq12
```

**Tại sao chọn BN254?**
1. **Pairing-friendly**: Embedding degree thấp (k=12) → pairing hiệu quả
2. **128-bit security**: Đủ mạnh cho hầu hết applications
3. **Wide support**: Được support trong nhiều libraries và hardware
4. **Efficient arithmetic**: Field size được chọn để optimize computer arithmetic

### Performance Analysis - Số liệu cụ thể

**Complexity của các operations:**

```cpp
// Field operations (256-bit)
Fr::add()         // ~1-2 CPU cycles
Fr::mul()         // ~50-100 CPU cycles (Montgomery)
Fr::inverse()     // ~50,000 CPU cycles (Extended Euclidean)

// Elliptic curve operations  
G1::add()         // ~500 CPU cycles
G1::double()      // ~400 CPU cycles
G1::scalar_mul()  // ~200,000 CPU cycles (256-bit scalar)

// Pairing operations
pairing()         // ~2,000,000 CPU cycles
multi_pairing()   // ~1,500,000 per additional pair

// Protocol operations (circuit size n)
setup()           // O(n log n) - ~10-60 seconds for n=2^20
prove()           // O(n log n) - ~5-30 seconds for n=2^20  
verify()          // O(1) - ~2-5 milliseconds regardless of n
```

**Memory usage:**
```cpp
sizeof(Fr)        // 32 bytes
sizeof(G1)        // 96 bytes (uncompressed), 48 bytes (compressed)
sizeof(G2)        // 192 bytes (uncompressed), 96 bytes (compressed)
sizeof(Proof)     // 256 bytes total (G1 + G2 + G1)

// For circuit with n constraints:
sizeof(ProvingKey)   // ~32n + 96n + 192n = ~320n bytes
sizeof(VerifyingKey) // ~96 * num_public_inputs bytes
```

### Security Considerations - Phân tích bảo mật

**Trusted Setup Ceremony:**
```
⚠️ CRITICAL SECURITY REQUIREMENTS:

1. Random generation:
   - α, β, γ, δ, τ phải thật sự random
   - Sử dụng cryptographically secure RNG
   - Collect entropy từ multiple sources

2. Toxic waste destruction:
   - Tất cả secret values phải bị overwrite
   - Memory pages phải được cleared
   - Hardware entropy sources phải được reset

3. Multi-party computation:
   - Trong production, cần MPC ceremony
   - Chỉ cần 1 party honest là đủ
   - Public verifiability của ceremony
```

**Attack vectors và defenses:**

1. **Setup compromise**: Nếu attacker biết τ → có thể fake proofs
   - **Defense**: Secure MPC ceremony với public verifiability

2. **Side-channel attacks**: Timing, power analysis có thể leak secrets
   - **Defense**: Constant-time implementations

3. **Implementation bugs**: Buffer overflows, memory corruption
   - **Defense**: Memory-safe languages, formal verification

4. **Quantum attacks**: Shor's algorithm phá discrete log
   - **Defense**: Post-quantum ZK-SNARKs (STARKs, lattice-based)

### Code Examples - Sử dụng thực tế

**Example 1: Age verification without revealing age**
```cpp
class AgeVerificationCircuit {
public:
    // Prove: age >= 18 without revealing exact age
    static R1CS create_r1cs() {
        R1CS r1cs;
        // Variables: [1, is_adult, age, age-18, comparison_bits...]
        
        // Constraint 1: age - 18 = difference
        // Constraint 2: difference in range [0, 100] using bit decomposition
        // Constraint 3: is_adult = (difference >= 0)
        
        return r1cs;
    }
    
    static auto create_witness(uint32_t age) {
        bool is_adult = (age >= 18);
        std::vector<Fr> public_inputs = {Fr(is_adult ? 1 : 0)};
        std::vector<Fr> private_inputs = {Fr(age), /* other intermediate values */};
        return std::make_pair(public_inputs, private_inputs);
    }
};

// Usage:
void prove_age_verification() {
    // Setup (chạy một lần)
    auto qap = QAP::from_r1cs(AgeVerificationCircuit::create_r1cs());
    auto crs = Groth16::setup(qap);
    
    // Alice proves she's adult without revealing age
    uint32_t alice_age = 25;  // Secret!
    auto [public_inputs, private_inputs] = 
        AgeVerificationCircuit::create_witness(alice_age);
    
    auto proof = Groth16::prove(crs.pk, qap, public_inputs, private_inputs);
    
    // Bob verifies: Alice is adult, but doesn't know her exact age
    bool is_valid = Groth16::verify(crs.vk, public_inputs, proof);
    std::cout << "Alice is adult: " << (public_inputs[0] == Fr(1)) << std::endl;
    std::cout << "Proof valid: " << is_valid << std::endl;
    // Alice's exact age remains secret!
}
```

**Example 2: Private voting**
```cpp
class VotingCircuit {
public:
    // Prove: "I have a valid voter ID and I'm voting for candidate X"
    // Without revealing voter ID
    static R1CS create_r1cs() {
        R1CS r1cs;
        // Variables: [1, vote, voter_id, merkle_path_elements...]
        
        // Constraint 1: voter_id is in Merkle tree (membership proof)
        // Constraint 2: vote ∈ {0, 1, 2} (valid candidate)
        // Constraint 3: Merkle path verification
        
        return r1cs;
    }
};
```

### Implementation Details - Chi tiết kỹ thuật

**Montgomery Arithmetic Explained:**
```cpp
// Tại sao Montgomery multiplication nhanh hơn?
// Thay vì tính (a * b) mod p trực tiếp, ta làm:

class Fr {
    static const uint64_t P[4];     // Prime modulus
    static const uint64_t R[4];     // 2^256 mod P  
    static const uint64_t R2[4];    // 2^512 mod P
    static const uint64_t INV;      // -P^(-1) mod 2^64
    
    uint64_t data[4];  // Always in Montgomery form: a * R mod P
    
    // Montgomery multiplication: CIOS method
    Fr montgomery_mul(const Fr& a, const Fr& b) const {
        uint64_t result[8] = {0};  // 512-bit temporary
        
        // Step 1: Multiply a.data * b.data
        for (int i = 0; i < 4; i++) {
            uint64_t carry = 0;
            for (int j = 0; j < 4; j++) {
                uint128_t prod = (uint128_t)a.data[i] * b.data[j] + result[i+j] + carry;
                result[i+j] = (uint64_t)prod;
                carry = prod >> 64;
            }
            result[i+4] = carry;
            
            // Step 2: Montgomery reduction
            uint64_t m = result[i] * INV;
            carry = 0;
            for (int j = 0; j < 4; j++) {
                uint128_t sum = (uint128_t)m * P[j] + result[i+j] + carry;
                result[i+j] = (uint64_t)sum;
                carry = sum >> 64;
            }
            // Propagate carry...
        }
        
        // Step 3: Final reduction if needed
        Fr output;
        if (compare_ge(result+4, P)) {
            subtract(result+4, P, output.data);
        } else {
            copy(result+4, output.data);
        }
        
        return output;
    }
};
```

**Elliptic Curve Point Addition:**
```cpp
// Addition formula trong Jacobian coordinates
// P = (X1, Y1, Z1), Q = (X2, Y2, Z2)
// R = P + Q = (X3, Y3, Z3)

G1 add_jacobian(const G1& P, const G1& Q) {
    if (P.is_zero()) return Q;
    if (Q.is_zero()) return P;
    
    // Tính các intermediate values
    Fq Z1Z1 = P.z * P.z;        // Z1²
    Fq Z2Z2 = Q.z * Q.z;        // Z2²
    Fq U1 = P.x * Z2Z2;         // X1 * Z2²
    Fq U2 = Q.x * Z1Z1;         // X2 * Z1²
    Fq S1 = P.y * Z2Z2 * Q.z;   // Y1 * Z2³
    Fq S2 = Q.y * Z1Z1 * P.z;   // Y2 * Z1³
    
    if (U1 == U2) {
        if (S1 == S2) {
            return double_jacobian(P);  // P == Q, use doubling
        } else {
            return G1::zero();          // P == -Q, result is infinity
        }
    }
    
    Fq H = U2 - U1;             // H = U2 - U1
    Fq I = (H + H) * (H + H);   // I = 4H²
    Fq J = H * I;               // J = H * I
    Fq r = S2 - S1;             // r = 2(S2 - S1)
    r = r + r;
    Fq V = U1 * I;              // V = U1 * I
    
    // Compute result
    Fq X3 = r * r - J - V - V;                    // X3 = r² - J - 2V
    Fq Y3 = r * (V - X3) - S1 * J * Fq(2);       // Y3 = r(V - X3) - 2*S1*J
    Fq Z3 = P.z * Q.z * H * Fq(2);               // Z3 = 2*Z1*Z2*H
    
    return G1(X3, Y3, Z3);
}
```

**Multi-Scalar Multiplication (MSM) - Pippenger's Algorithm:**
```cpp
// Efficient computation of k1*P1 + k2*P2 + ... + kn*Pn
G1 msm_pippenger(const std::vector<Fr>& scalars, 
                 const std::vector<G1>& points) {
    size_t n = scalars.size();
    if (n == 0) return G1::zero();
    if (n == 1) return scalars[0] * points[0];
    
    // Choose optimal window size
    size_t c = optimal_window_size(n);  // Typically 10-16 for large n
    size_t num_windows = (256 + c - 1) / c;
    
    G1 result = G1::zero();
    
    // Process from most significant window to least
    for (int i = num_windows - 1; i >= 0; i--) {
        // Accumulate previous windows
        for (size_t j = 0; j < c; j++) {
            result = result.double();
        }
        
        // Bucket method for current window
        std::vector<G1> buckets(1 << c);  // 2^c buckets
        
        for (size_t j = 0; j < n; j++) {
            // Extract c bits from position i*c
            uint32_t bucket_index = extract_window(scalars[j], i * c, c);
            if (bucket_index != 0) {
                buckets[bucket_index] += points[j];
            }
        }
        
        // Sum buckets efficiently
        G1 window_sum = G1::zero();
        G1 running_sum = G1::zero();
        
        for (int k = (1 << c) - 1; k > 0; k--) {
            running_sum += buckets[k];
            window_sum += running_sum;
        }
        
        result += window_sum;
    }
    
    return result;
}
```

### Mathematical Background - Nền tảng toán học

**Pairing Functions - Detailed Explanation:**
```
Bilinear pairing là một function e: G1 × G2 → GT với tính chất:
1. Bilinearity: e(aP, bQ) = e(P, Q)^(ab) = e(bP, aQ)
2. Non-degeneracy: Tồn tại P, Q sao cho e(P, Q) ≠ 1
3. Efficiency: Tính được trong polynomial time

Optimal Ate pairing trên BN254:
- Input: P ∈ G1, Q ∈ G2  
- Output: f ∈ Fq12

Miller Loop:
1. Initialize f = 1, T = Q
2. For each bit của loop parameter (từ MSB xuống LSB):
   a. f = f² * line_{T,T}(P)
   b. T = 2T
   c. If bit = 1:
      - f = f * line_{T,Q}(P) 
      - T = T + Q
3. Return f

Final Exponentiation:
f_final = f^((q^12 - 1)/r)

Chia làm 2 phases:
- Easy part: f^(q^6 - 1)
- Hard part: f^((q^6 + 1)/r) using cyclotomic subgroup
```

**R1CS to QAP Conversion - Step by step:**
```
Cho R1CS với m constraints và n variables:

Step 1: Setup domain
- Chọn m distinct values: r1, r2, ..., rm ∈ Fr
- Thường chọn powers of primitive root: r_i = ω^(i-1)

Step 2: Lagrange interpolation
For each variable j = 1..n:
- A_j(x) = Lagrange interpolation of points (r_i, a_{i,j})
- B_j(x) = Lagrange interpolation of points (r_i, b_{i,j})  
- C_j(x) = Lagrange interpolation of points (r_i, c_{i,j})

Công thức Lagrange:
A_j(x) = Σ(i=1 to m) a_{i,j} * L_i(x)

where L_i(x) = Π(k≠i) (x - r_k) / (r_i - r_k)

Step 3: Target polynomial
t(x) = Π(i=1 to m) (x - r_i)

Step 4: Verification
For satisfying assignment s = (s_1, ..., s_n):
A(x) = Σ s_j * A_j(x)
B(x) = Σ s_j * B_j(x)  
C(x) = Σ s_j * C_j(x)

Then: A(x) * B(x) - C(x) = h(x) * t(x) for some polynomial h(x)
```
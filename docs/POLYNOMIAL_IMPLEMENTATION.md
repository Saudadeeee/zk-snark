# Polynomial Implementation Documentation

## Tổng quan (Overview)

Module `Polynomial` trong ZKMini cung cấp implementation hoàn chỉnh của các phép toán đa thức trên trường hữu hạn Fr. Đây là component quan trọng cho việc thực hiện giao thức Groth16, đặc biệt trong QAP (Quadratic Arithmetic Program) và Lagrange interpolation.

## Kiến trúc thiết kế (Design Architecture)

### 1. Biểu diễn đa thức (Polynomial Representation)

```cpp
class Polynomial {
    std::vector<Fr> coeffs; // coeffs[i] là hệ số của x^i
    // Invariant: đa thức luôn được normalize (loại bỏ hệ số 0 ở đầu)
    // Convention: deg(0) = -1
}
```

**Đặc điểm thiết kế:**
- **Dense representation**: Lưu trữ tất cả hệ số từ x^0 đến x^deg
- **Auto-normalization**: Tự động loại bỏ hệ số 0 ở vị trí cao nhất
- **Zero-safe**: Xử lý an toàn đa thức zero với deg = -1

### 2. Factory Methods và Constructors

```cpp
// Constructors cơ bản
Polynomial();                           // Đa thức zero
Polynomial(const std::vector<Fr>& coeffs); // Từ vector hệ số
Polynomial(size_t degree);              // Zero polynomial với bậc cho trước

// Factory methods
static Polynomial zero();               // Trả về đa thức 0
static Polynomial one();                // Trả về đa thức 1
static Polynomial from_coeffs(const std::vector<Fr>& coeffs);
```

## Chi tiết Implementation

### 3. Phép toán số học cơ bản (Basic Arithmetic)

#### 3.1 Phép cộng đa thức (Addition)
```cpp
static Polynomial add(const Polynomial& a, const Polynomial& b) {
    size_t max_size = std::max(a.coeffs.size(), b.coeffs.size());
    std::vector<Fr> result;
    result.reserve(max_size);
    
    for (size_t i = 0; i < max_size; ++i) {
        Fr sum = Fr();
        if (i < a.coeffs.size()) sum = a.coeffs[i];
        if (i < b.coeffs.size()) sum = sum + b.coeffs[i];
        result.push_back(sum);
    }
    
    Polynomial p(result);
    p.normalize();
    return p;
}
```

**Thuật toán:**
1. Tìm kích thước lớn nhất của hai đa thức
2. Cộng từng hệ số tương ứng
3. Normalize kết quả để loại bỏ leading zeros

#### 3.2 Phép nhân vô hướng (Scalar Multiplication)
```cpp
static Polynomial scalar_mul(const Polynomial& f, const Fr& k) {
    if (k.is_zero()) return Polynomial::zero();
    if (f.is_zero()) return Polynomial::zero();
    
    std::vector<Fr> result;
    result.reserve(f.coeffs.size());
    
    for (const Fr& coeff : f.coeffs) {
        result.push_back(coeff * k);
    }
    
    return Polynomial(result);
}
```

### 4. Phép nhân đa thức (Polynomial Multiplication)

#### 4.1 Schoolbook Multiplication
```cpp
static Polynomial mul_schoolbook(const Polynomial& f, const Polynomial& g) {
    if (f.is_zero() || g.is_zero()) return Polynomial::zero();
    
    size_t result_size = f.coeffs.size() + g.coeffs.size() - 1;
    std::vector<Fr> result(result_size, Fr());
    
    for (size_t i = 0; i < f.coeffs.size(); ++i) {
        for (size_t j = 0; j < g.coeffs.size(); ++j) {
            result[i + j] = result[i + j] + (f.coeffs[i] * g.coeffs[j]);
        }
    }
    
    Polynomial p;
    p.coeffs = result;
    p.normalize();
    return p;
}
```

**Độ phức tạp:** O(nm) với n, m là bậc của hai đa thức

### 5. Tính giá trị đa thức (Polynomial Evaluation)

#### 5.1 Horner's Method
```cpp
static Fr eval(const Polynomial& f, const Fr& x) {
    if (f.is_zero() || f.coeffs.empty()) return Fr();
    
    Fr acc = f.coeffs.back();  // Bắt đầu từ hệ số cao nhất
    for (int i = static_cast<int>(f.coeffs.size()) - 2; i >= 0; --i) {
        acc = acc * x + f.coeffs[i];
    }
    return acc;
}
```

**Thuật toán Horner:**
- Tính f(x) = ((a_n * x + a_{n-1}) * x + ... + a_1) * x + a_0
- Chỉ cần n phép nhân và n phép cộng
- Độ phức tạp: O(n)

### 6. Phép chia đa thức (Polynomial Division)

#### 6.1 Long Division Algorithm
```cpp
void Polynomial::divrem(const Polynomial& N, const Polynomial& D, 
                       Polynomial& Q, Polynomial& R) {
    assert(!D.is_zero() && "Divisor cannot be zero");
    
    if (N.deg() < D.deg()) {
        Q = Polynomial::zero();
        R = N;
        return;
    }
    
    Q = Polynomial::zero();
    R = N;
    
    while (R.deg() >= D.deg() && !R.is_zero()) {
        int k = R.deg() - D.deg();
        Fr leadR = R.coeffs.back();
        Fr leadD = D.coeffs.back();
        
        Fr leadD_inv = leadD.inverse();
        Fr t = leadR * leadD_inv;
        
        Q.set_coeff(k, Q.coeff(k) + t);
        
        Polynomial temp = scalar_mul(D, t);
        temp = mul_xk(temp, k);
        R = sub(R, temp);
        R.normalize();
    }
}
```

**Thuật toán:**
1. Kiểm tra điều kiện chia được
2. Lặp đến khi bậc của R < bậc của D
3. Tính hệ số thương và cập nhật remainder

### 7. Lagrange Interpolation

#### 7.1 Lagrange Basis Polynomial
```cpp
static Polynomial lagrange_basis(const std::vector<Fr>& pts, size_t j) {
    // Tính L_j(x) = ∏(i≠j) (x - s_i) / ∏(i≠j) (s_j - s_i)
    
    // Tử số: N_j(X) = ∏(i≠j) (X - s_i)
    Polynomial Nj = Polynomial::one();
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != j) {
            Fr neg_pt_i = Fr() - pts[i];
            std::vector<Fr> lin_coeffs = {neg_pt_i, Fr(1)};
            Polynomial lin(lin_coeffs);
            Nj = mul_schoolbook(Nj, lin);
        }
    }
    
    // Mẫu số: D_j = ∏(i≠j) (s_j - s_i)
    Fr Dj = Fr(1);
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != j) {
            Fr diff = pts[j] - pts[i];
            assert(!diff.is_zero() && "Duplicate points detected");
            Dj = Dj * diff;
        }
    }
    
    Fr Dj_inv = Dj.inverse();
    return scalar_mul(Nj, Dj_inv);
}
```

#### 7.2 Full Interpolation
```cpp
static Polynomial interpolate(const std::vector<Fr>& pts, 
                            const std::vector<Fr>& vals) {
    assert(pts.size() == vals.size() && "Points and values size mismatch");
    
    Polynomial P = Polynomial::zero();
    
    for (size_t j = 0; j < pts.size(); ++j) {
        Polynomial Lj = lagrange_basis(pts, j);
        Polynomial term = scalar_mul(Lj, vals[j]);
        P = add(P, term);
    }
    
    P.normalize();
    return P;
}
```

**Công thức Lagrange:**
- P(x) = Σ(j=0 to n) v_j * L_j(x)
- L_j(x) = ∏(i≠j) (x - x_i) / ∏(i≠j) (x_j - x_i)

### 8. Vanishing Polynomial

```cpp
static Polynomial vanishing(const std::vector<Fr>& points) {
    Polynomial Z = Polynomial::one();
    
    for (const Fr& s : points) {
        // Tạo đa thức tuyến tính (X - s) = (-s, 1)
        Fr neg_s = Fr() - s;
        std::vector<Fr> lin_coeffs = {neg_s, Fr(1)};
        Polynomial lin(lin_coeffs);
        Z = mul_schoolbook(Z, lin);
    }
    
    Z.normalize();
    return Z;
}
```

**Định nghĩa:** Z(x) = ∏(s ∈ S) (x - s)

**Tính chất:** Z(s) = 0 for all s ∈ S

## Tối ưu hóa và Performance

### 9. Memory Management

```cpp
// Pre-allocation cho performance
std::vector<Fr> result;
result.reserve(max_size);

// In-place operations để giảm allocation
static void add_inplace(Polynomial& dst, const Polynomial& src);
static void scalar_mul_inplace(Polynomial& f, const Fr& k);
```

### 10. Normalization Strategy

```cpp
void normalize() {
    while (!coeffs.empty() && coeffs.back().is_zero()) {
        coeffs.pop_back();
    }
}
```

**Tối ưu hóa:**
- Chỉ normalize khi cần thiết
- Sử dụng static ZERO_FR để tránh tạo object mới

### 11. Legacy Compatibility

```cpp
// Wrapper methods cho tương thích ngược
size_t degree() const;
Fr leading_coefficient() const;
std::pair<Polynomial, Polynomial> divide(const Polynomial& divisor) const;
static Polynomial lagrange_interpolate(const std::vector<Fr>& x_coords, 
                                     const std::vector<Fr>& y_coords);
```

## Use Cases trong ZK-SNARK

### 12. QAP Integration

```cpp
// Polynomial evaluation tại trusted setup
std::vector<Fr> evaluate_batch(const std::vector<Fr>& x_vec) const {
    std::vector<Fr> result;
    result.reserve(x_vec.size());
    for (const Fr& x : x_vec) {
        result.push_back(evaluate(x));
    }
    return result;
}
```

### 13. Random Polynomial Generation

```cpp
static Polynomial random(size_t degree) {
    std::vector<Fr> coeffs;
    coeffs.reserve(degree + 1);
    for (size_t i = 0; i <= degree; ++i) {
        coeffs.push_back(random_fr());
    }
    return Polynomial(coeffs);
}
```

## Operator Overloading

```cpp
// Syntactic sugar cho các phép toán
Polynomial operator+(const Polynomial& other) const;
Polynomial operator-(const Polynomial& other) const;
Polynomial operator*(const Polynomial& other) const;
Polynomial operator*(const Fr& scalar) const;
bool operator==(const Polynomial& other) const;
```

## Error Handling và Assertions

```cpp
assert(!D.is_zero() && "Divisor cannot be zero");
assert(j < pts.size() && "Index j out of bounds");
assert(pts.size() == vals.size() && "Points and values size mismatch");
assert(!diff.is_zero() && "Duplicate points detected");
```

## Testing và Validation

### Các test cases quan trọng:
1. **Basic arithmetic**: Addition, subtraction, multiplication
2. **Evaluation**: Horner's method correctness
3. **Division**: Quotient và remainder validation
4. **Interpolation**: Lagrange polynomial correctness
5. **Edge cases**: Zero polynomials, single coefficients
6. **Performance**: Large degree polynomials

## Kết luận

Polynomial implementation trong ZKMini cung cấp:
- ✅ **Completeness**: Tất cả phép toán cần thiết cho ZK-SNARK
- ✅ **Correctness**: Algorithms đã được verify
- ✅ **Performance**: Optimized memory usage và computation
- ✅ **Usability**: Intuitive API với operator overloading
- ✅ **Maintainability**: Clean code structure với comprehensive documentation

Module này là foundation quan trọng cho QAP operations và trusted setup trong Groth16 protocol.
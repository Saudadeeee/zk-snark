#include "zkmini/pairing.hpp"
#include "zkmini/utils.hpp"

namespace zkmini {

Fq12 Pairing::pairing(const G1& P, const G2& Q) {
    // Optimal Ate pairing implementation for BN254
    Fq12 result = miller_loop(P, Q);
    return result.final_exponentiation();
}

Fq12 Pairing::miller_loop(const G1& P, const G2& Q) {
    if (P.is_zero() || Q.is_zero()) {
        return Fq12(); // Return 1 for zero points
    }
    
    Fq12 f; // Initialize to 1
    G2 T = Q; // Working copy
    
    // BN254 curve parameter (simplified implementation)
    // For full implementation, use the actual BN curve parameter
    uint64_t loop_count = 0x44E992B44A6909F1; // BN254 parameter
    
    // Miller loop with binary representation
    for (int i = 62; i >= 0; --i) { // 63-bit loop count
        f = f * f * line_double(T, P);
        
        if ((loop_count >> i) & 1) {
            f = f * line_add(T, Q, P);
        }
    }
    
    return f;
}

Fq12 Pairing::multi_pairing(const std::vector<G1>& P_vec, const std::vector<G2>& Q_vec) {
    ZK_ASSERT(P_vec.size() == Q_vec.size(), "P and Q vectors must have same size");
    
    if (P_vec.empty()) {
        return Fq12(); // Return 1
    }
    
    // Accumulate Miller loops
    Fq12 result;
    for (size_t i = 0; i < P_vec.size(); ++i) {
        Fq12 loop_result = miller_loop(P_vec[i], Q_vec[i]);
        if (i == 0) {
            result = loop_result;
        } else {
            result = result * loop_result;
        }
    }
    
    // Apply final exponentiation once
    return result.final_exponentiation();
}

bool Pairing::pairing_check(const std::vector<G1>& P_vec, const std::vector<G2>& Q_vec) {
    Fq12 result = multi_pairing(P_vec, Q_vec);
    return result.is_one();
}

Pairing::G2Prepared Pairing::prepare_g2(const G2& Q) {
    G2Prepared result;
    result.point = Q;
    
    if (Q.is_zero()) {
        return result;
    }
    
    // Precompute some values for optimization
    // In a full implementation, this would precompute line functions
    // For now, just store the point and initialize empty lines vector
    result.lines.clear();
    
    return result;
}

Fq12 Pairing::miller_loop_prepared(const G1& P, const G2Prepared& Q_prep) {
    if (P.is_zero() || Q_prep.point.is_zero()) {
        return Fq12(); // Return 1
    }
    
    // Use precomputed values for optimization
    return miller_loop(P, Q_prep.point);
}

Fq12 Pairing::line_double(G2& R, const G1& P) {
    if (R.is_zero()) {
        return Fq12(); // Return 1
    }
    
    // Get affine coordinates
    auto [Rx, Ry] = R.to_affine();
    auto [Px, Py] = P.to_affine();
    
    // Compute tangent slope: lambda = (3*Rx^2) / (2*Ry)
    Fq2 three_Rx2 = Rx * Rx * Fq2(Fq(3), Fq());
    Fq2 two_Ry = Ry + Ry;
    Fq2 lambda = three_Rx2 * two_Ry.inverse();
    
    // Line function: l = lambda * (Px - Rx) - (Py - Ry)
    // Embedding into Fq12
    Fq12 line_value;
    line_value.c0.c0.c0 = lambda.c0 * (Px - Rx.c0) - (Py - Ry.c0);
    line_value.c0.c0.c1 = lambda.c1 * (Px - Rx.c0) - (Fq() - Ry.c1);
    
    // Update R = 2R
    R = R.double_point();
    
    return line_value;
}

Fq12 Pairing::line_add(G2& R, const G2& Q, const G1& P) {
    if (R.is_zero()) {
        R = Q;
        return Fq12(); // Return 1
    }
    if (Q.is_zero()) {
        return Fq12(); // Return 1
    }
    
    // Get affine coordinates
    auto [Rx, Ry] = R.to_affine();
    auto [Qx, Qy] = Q.to_affine();
    auto [Px, Py] = P.to_affine();
    
    // Check if points are the same
    if (Rx == Qx && Ry == Qy) {
        return line_double(R, P);
    }
    
    // Compute slope: lambda = (Qy - Ry) / (Qx - Rx)
    Fq2 delta_y = Qy - Ry;
    Fq2 delta_x = Qx - Rx;
    Fq2 lambda = delta_y * delta_x.inverse();
    
    // Line function: l = lambda * (Px - Rx) - (Py - Ry)
    // Embedding into Fq12
    Fq12 line_value;
    line_value.c0.c0.c0 = lambda.c0 * (Px - Rx.c0) - (Py - Ry.c0);
    line_value.c0.c0.c1 = lambda.c1 * (Px - Rx.c0) - (Fq() - Ry.c1);
    
    // Update R = R + Q
    R = R + Q;
    
    return line_value;
}

} // namespace zkmini
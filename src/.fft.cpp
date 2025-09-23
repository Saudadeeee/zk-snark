#include "zkmini/.fft.hpp"
#include "zkmini/.utils.hpp"

namespace zkmini {

FFT::FFT(size_t domain_size) : domain_size(domain_size) {
    ZK_ASSERT(BitUtils::is_power_of_two(domain_size), "Domain size must be power of 2");
    root_of_unity = find_root_of_unity(domain_size);
    inv_root_of_unity = root_of_unity.inverse();
    compute_domain();
    compute_twiddles();
}

std::vector<Fr> FFT::fft(const std::vector<Fr>& coeffs) const {
    std::vector<Fr> result = coeffs;
    result.resize(domain_size, Fr());
    fft_in_place(result, false);
    return result;
}

std::vector<Fr> FFT::ifft(const std::vector<Fr>& evals) const {
    std::vector<Fr> result = evals;
    ZK_ASSERT(result.size() == domain_size, "Evaluation vector size mismatch");
    fft_in_place(result, true);
    
    // Scale by 1/n
    Fr inv_n = Fr(domain_size).inverse();
    for (Fr& val : result) {
        val = val * inv_n;
    }
    return result;
}

Polynomial FFT::multiply(const Polynomial& a, const Polynomial& b) {
    // TODO: Implement FFT-based polynomial multiplication
    return Polynomial();
}

std::vector<Fr> FFT::evaluate_on_domain(const Polynomial& poly) const {
    return fft(poly.coeffs);
}

Polynomial FFT::interpolate_on_domain(const std::vector<Fr>& evals) const {
    std::vector<Fr> coeffs = ifft(evals);
    return Polynomial(coeffs);
}

Fr FFT::get_root_of_unity(size_t i) const {
    ZK_ASSERT(i < domain_size, "Index out of bounds");
    return domain[i];
}

void FFT::compute_domain() {
    domain.clear();
    domain.reserve(domain_size);
    
    Fr current = Fr(1);
    for (size_t i = 0; i < domain_size; ++i) {
        domain.push_back(current);
        current = current * root_of_unity;
    }
}

void FFT::compute_twiddles() {
    // TODO: Precompute twiddle factors for optimization
}

void FFT::fft_in_place(std::vector<Fr>& a, bool inverse) const {
    // TODO: Implement in-place Cooley-Tukey FFT
    bit_reverse(a);
    
    Fr w = inverse ? inv_root_of_unity : root_of_unity;
    
    for (size_t len = 2; len <= domain_size; len <<= 1) {
        Fr wlen = w;
        for (size_t i = len; i < domain_size; i <<= 1) {
            wlen = wlen * wlen;
        }
        
        for (size_t i = 0; i < domain_size; i += len) {
            Fr w_current = Fr(1);
            for (size_t j = 0; j < len / 2; ++j) {
                Fr u = a[i + j];
                Fr v = a[i + j + len / 2] * w_current;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w_current = w_current * wlen;
            }
        }
    }
}

void FFT::bit_reverse(std::vector<Fr>& a) const {
    // TODO: Implement bit reversal permutation
    size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }
}

Fr FFT::find_root_of_unity(size_t n) {
    // TODO: Find primitive n-th root of unity in Fr
    return Fr();
}

} // namespace zkmini
#pragma once

#include "field.hpp"
#include "polynomial.hpp"
#include <vector>

namespace zkmini {

// Placeholder for Fast Fourier Transform operations
// TODO: Implement NTT/FFT for polynomial multiplication
class FFT {
public:
    // Initialize FFT with domain size (must be power of 2)
    FFT(size_t domain_size);
    
    // Forward FFT: coefficients -> evaluations
    std::vector<Fr> fft(const std::vector<Fr>& coeffs) const;
    
    // Inverse FFT: evaluations -> coefficients  
    std::vector<Fr> ifft(const std::vector<Fr>& evals) const;
    
    // Polynomial multiplication using FFT
    static Polynomial multiply(const Polynomial& a, const Polynomial& b);
    
    // Batch evaluation at all n-th roots of unity
    std::vector<Fr> evaluate_on_domain(const Polynomial& poly) const;
    
    // Interpolate from evaluations on domain
    Polynomial interpolate_on_domain(const std::vector<Fr>& evals) const;
    
    // Get the i-th root of unity
    Fr get_root_of_unity(size_t i) const;
    
    // Get all roots of unity
    const std::vector<Fr>& get_domain() const { return domain; }
    
    size_t size() const { return domain_size; }

private:
    size_t domain_size;
    Fr root_of_unity;      // primitive n-th root of unity
    Fr inv_root_of_unity;  // inverse of root_of_unity
    std::vector<Fr> domain; // all n-th roots of unity
    
    // Precomputed twiddle factors for optimization
    std::vector<Fr> twiddles;
    std::vector<Fr> inv_twiddles;
    
    void compute_domain();
    void compute_twiddles();
    
    // Core FFT algorithms
    void fft_in_place(std::vector<Fr>& a, bool inverse) const;
    void bit_reverse(std::vector<Fr>& a) const;
    
    // Find primitive root of unity
    static Fr find_root_of_unity(size_t n);
};

} // namespace zkmini
#pragma once

#include "field.hpp"
#include "polynomial.hpp"
#include <vector>

namespace zkmini {
class FFT {
public:
    
    FFT(size_t domain_size);
    
    
    std::vector<Fr> fft(const std::vector<Fr>& coeffs) const;
    
    
    std::vector<Fr> ifft(const std::vector<Fr>& evals) const;
    
    
    static Polynomial multiply(const Polynomial& a, const Polynomial& b);
    
    
    std::vector<Fr> evaluate_on_domain(const Polynomial& poly) const;
    
    
    Polynomial interpolate_on_domain(const std::vector<Fr>& evals) const;
    
    
    Fr get_root_of_unity(size_t i) const;
    
    
    const std::vector<Fr>& get_domain() const { return domain; }
    
    size_t size() const { return domain_size; }

private:
    size_t domain_size;
    Fr root_of_unity;      
    Fr inv_root_of_unity;  
    std::vector<Fr> domain; 
    
    
    std::vector<Fr> twiddles;
    std::vector<Fr> inv_twiddles;
    
    void compute_domain();
    void compute_twiddles();
    
    
    void fft_in_place(std::vector<Fr>& a, bool inverse) const;
    void bit_reverse(std::vector<Fr>& a) const;
    
    
    static Fr find_root_of_unity(size_t n);
};

}
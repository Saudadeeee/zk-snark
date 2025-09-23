#pragma once

#include ".g1.hpp"
#include ".g2.hpp"
#include "field.hpp"
#include <vector>

namespace zkmini {

// Placeholder for Multi-Scalar Multiplication (MSM)
// TODO: Implement efficient MSM algorithms
class MSM {
public:
    // G1 MSM: Σ scalar_i * point_i
    static G1 msm_g1(const std::vector<Fr>& scalars, const std::vector<G1>& points);
    
    // G2 MSM: Σ scalar_i * point_i  
    static G2 msm_g2(const std::vector<Fr>& scalars, const std::vector<G2>& points);
    
    // Windowed MSM (more efficient for large inputs)
    static G1 windowed_msm_g1(const std::vector<Fr>& scalars, 
                              const std::vector<G1>& points, 
                              size_t window_size = 4);
    
    static G2 windowed_msm_g2(const std::vector<Fr>& scalars, 
                              const std::vector<G2>& points,
                              size_t window_size = 4);
    
    // Pippenger's algorithm (best for very large inputs)
    static G1 pippenger_msm_g1(const std::vector<Fr>& scalars, 
                               const std::vector<G1>& points);
    
    static G2 pippenger_msm_g2(const std::vector<Fr>& scalars, 
                               const std::vector<G2>& points);
    
    // Precomputed tables for fixed base MSM
    class G1Table {
    public:
        G1Table(const G1& base, size_t table_size);
        G1 multiply(const Fr& scalar) const;
    private:
        std::vector<G1> table;
        size_t window_size;
    };
    
    class G2Table {
    public:
        G2Table(const G2& base, size_t table_size);
        G2 multiply(const Fr& scalar) const;
    private:
        std::vector<G2> table;
        size_t window_size;
    };

private:
    // Helper functions
    static size_t optimal_window_size(size_t num_points);
    static std::vector<std::vector<size_t>> bucket_sort(const std::vector<Fr>& scalars, 
                                                        size_t window_size);
};

} // namespace zkmini
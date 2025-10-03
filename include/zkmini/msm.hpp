#pragma once

#include "g1.hpp"
#include "g2.hpp"
#include "field.hpp"
#include <vector>

namespace zkmini {

class MSM {
public:
    static G1 msm_g1(const std::vector<Fr>& scalars, const std::vector<G1>& points);
    
    static G2 msm_g2(const std::vector<Fr>& scalars, const std::vector<G2>& points);
    
    static G1 windowed_msm_g1(const std::vector<Fr>& scalars, 
                              const std::vector<G1>& points, 
                              size_t window_size = 4);
    
    static G2 windowed_msm_g2(const std::vector<Fr>& scalars, 
                              const std::vector<G2>& points,
                              size_t window_size = 4);
    
    static G1 pippenger_msm_g1(const std::vector<Fr>& scalars, 
                               const std::vector<G1>& points);
    
    static G2 pippenger_msm_g2(const std::vector<Fr>& scalars, 
                               const std::vector<G2>& points);
    
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
    static size_t optimal_window_size(size_t num_points);
    static std::vector<std::vector<size_t>> bucket_sort(const std::vector<Fr>& scalars, 
                                                        size_t window_size);
};

}
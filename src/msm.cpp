#include "zkmini/msm.hpp"
#include "zkmini/utils.hpp"

namespace zkmini {

G1 MSM::msm_g1(const std::vector<Fr>& scalars, const std::vector<G1>& points) {
    ZK_ASSERT(scalars.size() == points.size(), "Scalar and point vectors must have same size");
    
    G1 result;
    for (size_t i = 0; i < scalars.size(); ++i) {
        result = result + (points[i] * scalars[i]);
    }
    return result;
}

G2 MSM::msm_g2(const std::vector<Fr>& scalars, const std::vector<G2>& points) {
    ZK_ASSERT(scalars.size() == points.size(), "Scalar and point vectors must have same size");
    
    G2 result;
    for (size_t i = 0; i < scalars.size(); ++i) {
        result = result + (points[i] * scalars[i]);
    }
    return result;
}

G1 MSM::windowed_msm_g1(const std::vector<Fr>& scalars, 
                        const std::vector<G1>& points, 
                        size_t window_size) {
    ZK_ASSERT(scalars.size() == points.size(), "Scalar and point vectors must have same size");
    if (scalars.empty()) return G1();
    
    if (window_size == 0) {
        window_size = optimal_window_size(scalars.size());
    }
    
    G1 result;
    size_t num_windows = (256 + window_size - 1) / window_size;
    
    for (size_t window = 0; window < num_windows; ++window) {
        result = result.double_point();
        for (size_t bit = 0; bit < window_size; ++bit) {
            result = result.double_point();
        }
        
        
        for (size_t i = 0; i < scalars.size(); ++i) {
            auto scalar_bytes = scalars[i].to_bytes();
            size_t bit_index = (num_windows - 1 - window) * window_size;
            
            uint64_t window_value = 0;
            for (size_t bit = 0; bit < window_size && bit_index + bit < 256; ++bit) {
                size_t byte_idx = (bit_index + bit) / 8;
                size_t bit_in_byte = (bit_index + bit) % 8;
                
                if (byte_idx < scalar_bytes.size()) {
                    if (scalar_bytes[byte_idx] & (1 << bit_in_byte)) {
                        window_value |= (1ULL << bit);
                    }
                }
            }
            
            if (window_value > 0) {
                G1 temp = points[i];
                for (uint64_t j = 1; j < window_value; ++j) {
                    temp = temp + points[i];
                }
                result = result + temp;
            }
        }
    }
    
    return result;
}

G2 MSM::windowed_msm_g2(const std::vector<Fr>& scalars, 
                        const std::vector<G2>& points,
                        size_t window_size) {
    ZK_ASSERT(scalars.size() == points.size(), "Scalar and point vectors must have same size");
    if (scalars.empty()) return G2();
    
    if (window_size == 0) {
        window_size = optimal_window_size(scalars.size());
    }
    
    G2 result;
    size_t num_windows = (256 + window_size - 1) / window_size; 
    
    for (size_t window = 0; window < num_windows; ++window) {
        result = result.double_point();
        for (size_t bit = 0; bit < window_size; ++bit) {
            result = result.double_point();
        }
        
        
        for (size_t i = 0; i < scalars.size(); ++i) {
            auto scalar_bytes = scalars[i].to_bytes();
            size_t bit_index = (num_windows - 1 - window) * window_size;
            
            uint64_t window_value = 0;
            for (size_t bit = 0; bit < window_size && bit_index + bit < 256; ++bit) {
                size_t byte_idx = (bit_index + bit) / 8;
                size_t bit_in_byte = (bit_index + bit) % 8;
                
                if (byte_idx < scalar_bytes.size()) {
                    if (scalar_bytes[byte_idx] & (1 << bit_in_byte)) {
                        window_value |= (1ULL << bit);
                    }
                }
            }
            
            if (window_value > 0) {
                G2 temp = points[i];
                for (uint64_t j = 1; j < window_value; ++j) {
                    temp = temp + points[i];
                }
                result = result + temp;
            }
        }
    }
    
    return result;
}

G1 MSM::pippenger_msm_g1(const std::vector<Fr>& scalars, 
                         const std::vector<G1>& points) {
    return msm_g1(scalars, points);
}

G2 MSM::pippenger_msm_g2(const std::vector<Fr>& scalars, 
                         const std::vector<G2>& points) {
    return msm_g2(scalars, points);
}

MSM::G1Table::G1Table(const G1& base, size_t table_size) : window_size(4) {
    table.resize(1 << window_size);
    table[0] = G1(); 
    if (table_size > 1) {
        table[1] = base;
        for (size_t i = 2; i < table.size(); ++i) {
            table[i] = table[i-1] + base;
        }
    }
}

G1 MSM::G1Table::multiply(const Fr& scalar) const {
    if (scalar.is_zero()) return G1();
    
    G1 result;
    auto scalar_bytes = scalar.to_bytes();
    
    for (size_t byte_idx = 0; byte_idx < scalar_bytes.size(); ++byte_idx) {
        uint8_t byte = scalar_bytes[byte_idx];
        for (size_t bit = 0; bit < 8; bit += window_size) {
            
            uint64_t window_value = 0;
            for (size_t w = 0; w < window_size && bit + w < 8; ++w) {
                if (byte & (1 << (bit + w))) {
                    window_value |= (1ULL << w);
                }
            }
            
            
            for (size_t i = 0; i < window_size; ++i) {
                result = result.double_point();
            }
            
            
            if (window_value < table.size()) {
                result = result + table[window_value];
            }
        }
    }
    
    return result;
}

MSM::G2Table::G2Table(const G2& base, size_t table_size) : window_size(4) {
    table.resize(1 << window_size);
    table[0] = G2(); 
    if (table_size > 1) {
        table[1] = base;
        for (size_t i = 2; i < table.size(); ++i) {
            table[i] = table[i-1] + base;
        }
    }
}

G2 MSM::G2Table::multiply(const Fr& scalar) const {
    if (scalar.is_zero()) return G2();
    
    G2 result;
    auto scalar_bytes = scalar.to_bytes();
    
    for (size_t byte_idx = 0; byte_idx < scalar_bytes.size(); ++byte_idx) {
        uint8_t byte = scalar_bytes[byte_idx];
        for (size_t bit = 0; bit < 8; bit += window_size) {
            
            uint64_t window_value = 0;
            for (size_t w = 0; w < window_size && bit + w < 8; ++w) {
                if (byte & (1 << (bit + w))) {
                    window_value |= (1ULL << w);
                }
            }
            
            
            for (size_t i = 0; i < window_size; ++i) {
                result = result.double_point();
            }
            
            
            if (window_value < table.size()) {
                result = result + table[window_value];
            }
        }
    }
    
    return result;
}

size_t MSM::optimal_window_size(size_t num_points) {
    
    
    if (num_points < 8) return 2;
    if (num_points < 32) return 3;
    if (num_points < 128) return 4;
    if (num_points < 512) return 5;
    if (num_points < 2048) return 6;
    if (num_points < 8192) return 7;
    return 8; 
}

std::vector<std::vector<size_t>> MSM::bucket_sort(const std::vector<Fr>& scalars, 
                                                  size_t window_size) {
    return {};
}

}
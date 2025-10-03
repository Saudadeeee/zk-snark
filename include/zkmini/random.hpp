#pragma once

#include "field.hpp"
#include <random>
#include <vector>

namespace zkmini {
class Random {
public:
    
    Random(uint64_t seed = 0);
    
    
    Fr random_fr();
    
    
    std::vector<Fr> random_fr_vector(size_t count);
    
    
    std::vector<uint8_t> random_bytes(size_t count);
    
    
    uint64_t random_uint64(uint64_t min = 0, uint64_t max = UINT64_MAX);
    
    
    bool random_bool();
    
    
    std::vector<size_t> sample_indices(size_t total_count, size_t sample_size);
    
    
    template<typename T>
    void shuffle(std::vector<T>& vec);
    
    
    static Random& get_global();
    
    
    static void seed_global(uint64_t seed);

private:
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> uint64_dist;
    std::uniform_int_distribution<uint8_t> byte_dist;
    
    
    uint64_t get_system_seed();
};
Fr random_fr();
std::vector<Fr> random_fr_vector(size_t count);
std::vector<uint8_t> random_bytes(size_t count);
uint64_t random_uint64(uint64_t min = 0, uint64_t max = UINT64_MAX);
bool random_bool();
void seed_random(uint64_t seed);

}
#pragma once

#include "field.hpp"
#include <random>
#include <vector>

namespace zkmini {

// Placeholder for random number generation
// TODO: Implement cryptographically secure random number generation
class Random {
public:
    // Initialize with seed (for reproducible testing)
    Random(uint64_t seed = 0);
    
    // Generate random field element
    Fr random_fr();
    
    // Generate random field elements
    std::vector<Fr> random_fr_vector(size_t count);
    
    // Generate random bytes
    std::vector<uint8_t> random_bytes(size_t count);
    
    // Generate random integer in range [min, max)
    uint64_t random_uint64(uint64_t min = 0, uint64_t max = UINT64_MAX);
    
    // Generate random boolean
    bool random_bool();
    
    // Sample random subset of indices
    std::vector<size_t> sample_indices(size_t total_count, size_t sample_size);
    
    // Shuffle vector in place
    template<typename T>
    void shuffle(std::vector<T>& vec);
    
    // Global random instance (thread-local)
    static Random& get_global();
    
    // Seed global random
    static void seed_global(uint64_t seed);

private:
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> uint64_dist;
    std::uniform_int_distribution<uint8_t> byte_dist;
    
    // Use system entropy for seeding if seed=0
    uint64_t get_system_seed();
};

// Convenience functions using global random
Fr random_fr();
std::vector<Fr> random_fr_vector(size_t count);
std::vector<uint8_t> random_bytes(size_t count);
uint64_t random_uint64(uint64_t min = 0, uint64_t max = UINT64_MAX);
bool random_bool();

// Seed global random
void seed_random(uint64_t seed);

} // namespace zkmini
#include "zkmini/random.hpp"
#include <random>
#include <chrono>
#include <algorithm>

namespace zkmini {

// Random class implementation
Random::Random(uint64_t seed) : uint64_dist(0, UINT64_MAX), byte_dist(0, 255) {
    if (seed == 0) {
        seed = get_system_seed();
    }
    rng.seed(seed);
}

Fr Random::random_fr() {
    // Generate random field element in range [0, Fr::MODULUS)
    if constexpr (Fr::USE_64BIT_DEV) {
        // For development mode, use simple 64-bit generation
        uint64_t val = uint64_dist(rng);
        if (val >= Fr::MODULUS) {
            val %= Fr::MODULUS;
        }
        return Fr(val);
    } else {
        // For full 256-bit mode, generate proper random element
        std::array<uint64_t, 4> limbs;
        do {
            for (int i = 0; i < 4; i++) {
                limbs[i] = uint64_dist(rng);
            }
            // Reduce if >= modulus
        } while (limbs[3] > bn254_fr::MODULUS_BN254[3] || 
                (limbs[3] == bn254_fr::MODULUS_BN254[3] && 
                 (limbs[2] > bn254_fr::MODULUS_BN254[2] ||
                  (limbs[2] == bn254_fr::MODULUS_BN254[2] &&
                   (limbs[1] > bn254_fr::MODULUS_BN254[1] ||
                    (limbs[1] == bn254_fr::MODULUS_BN254[1] &&
                     limbs[0] >= bn254_fr::MODULUS_BN254[0]))))));
        
        return Fr(limbs);
    }
}

std::vector<Fr> Random::random_fr_vector(size_t count) {
    std::vector<Fr> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(random_fr());
    }
    return result;
}

std::vector<uint8_t> Random::random_bytes(size_t count) {
    std::vector<uint8_t> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(byte_dist(rng));
    }
    return result;
}

uint64_t Random::random_uint64(uint64_t min, uint64_t max) {
    if (min >= max) return min;
    std::uniform_int_distribution<uint64_t> dist(min, max - 1);
    return dist(rng);
}

bool Random::random_bool() {
    return uint64_dist(rng) % 2 == 0;
}

std::vector<size_t> Random::sample_indices(size_t total_count, size_t sample_size) {
    if (sample_size >= total_count) {
        std::vector<size_t> result(total_count);
        std::iota(result.begin(), result.end(), 0);
        return result;
    }
    
    std::vector<size_t> indices(total_count);
    std::iota(indices.begin(), indices.end(), 0);
    shuffle(indices);
    
    indices.resize(sample_size);
    return indices;
}

template<typename T>
void Random::shuffle(std::vector<T>& vec) {
    std::shuffle(vec.begin(), vec.end(), rng);
}

thread_local Random global_random;

Random& Random::get_global() {
    return global_random;
}

void Random::seed_global(uint64_t seed) {
    global_random = Random(seed);
}

uint64_t Random::get_system_seed() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

// Global convenience functions
Fr random_fr() {
    return Random::get_global().random_fr();
}

std::vector<Fr> random_fr_vector(size_t count) {
    return Random::get_global().random_fr_vector(count);
}

std::vector<uint8_t> random_bytes(size_t count) {
    return Random::get_global().random_bytes(count);
}

uint64_t random_uint64(uint64_t min, uint64_t max) {
    return Random::get_global().random_uint64(min, max);
}

bool random_bool() {
    return Random::get_global().random_bool();
}

void seed_random(uint64_t seed) {
    Random::seed_global(seed);
}

} // namespace zkmini
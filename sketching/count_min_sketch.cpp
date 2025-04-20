#include "sketch.hpp"
#include "../hashutil.h"
#include <cmath>
#include <limits>


CountMinSketch::CountMinSketch(uint64_t t, uint64_t k) : m(0), t(t), k(k) {
    this->table = (uint64_t*) malloc(t * k * sizeof(uint64_t));
    this->hash_coeffs = (uint64_t*) malloc(t * 2 * sizeof(uint64_t));


    // coefficients for t pairwise independent hash functions

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distrib_a(1ULL, LARGE_PRIME - 1ULL); // 0 < a < p
    std::uniform_int_distribution<uint64_t> distrib_b(0ULL, LARGE_PRIME - 1ULL); // 0 ≤ b < p

    for (uint64_t i = 0; i < t; i++) {
        hash_coeffs[i * 2] = distrib_a(gen);
        hash_coeffs[i * 2 + 1] = distrib_b(gen);
    }
    
}

CountMinSketch::~CountMinSketch() {
    free(this->table);
    this->table = nullptr;

    free(this->hash_coeffs);
    this->hash_coeffs = nullptr;
}

inline uint64_t CountMinSketch::BucketHash(uint64_t x, uint64_t row) {
    uint64_t a = hash_coeffs[row * 2];
    uint64_t b = hash_coeffs[row * 2 + 1];
    __uint128_t t = (__uint128_t)a * x + b;
    return (uint64_t)(t % LARGE_PRIME) % this->k;
}

void CountMinSketch::Add(uint64_t x) {
    for (uint64_t row = 0; row < this->t; row++) {
        uint64_t bucket = BucketHash(x, row);
        table[row * this->k + bucket]++;
    }

    this->m++;

    this->seen.insert(x);
}

// min of t hashed counters
uint64_t CountMinSketch::Estimate(uint64_t x) {
    uint64_t min = UINT64_MAX;
    for (uint64_t row = 0; row < this->t; row++) {
        uint64_t bucket = BucketHash(x, row);
        min = std::min(min, table[row * this->k + bucket]);
    }
    return min;
}

std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> CountMinSketch::HeavyHitters(double phi) {
    uint64_t threshold = phi * this->m;
    
    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> hh;
    for (uint64_t x : seen) {
        uint64_t count = Estimate(x);
        if (count >= threshold) {
            hh.insert({count, x});
        }
    }

    return hh;
}

size_t CountMinSketch::Size() {
    return sizeof(*this) + (t * k + 3 + this->seen.size()) * sizeof(uint64_t);
    // + this->seen.size()
}

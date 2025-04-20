#include "sketch.hpp"
#include "../hashutil.h"
#include <cmath>
#include <limits>


CountMinSketch::CountMinSketch(uint64_t t, uint64_t k) : m(0), t(t), k(k) {
    this->table = (uint64_t*) malloc(t * k * sizeof(uint64_t));
}

CountMinSketch::~CountMinSketch() {
    free(this->table);
    this->table = nullptr;
}

inline uint64_t CountMinSketch::BucketHash(uint64_t x, uint64_t row) {
    return MurmurHash64A(&x, sizeof(x), row) % this->k;
}

void CountMinSketch::Add(uint64_t x) {
    for (uint64_t row = 0; row < this->t; row++) {
        // use row as hash seed // TODO: maybe store an array of salts, or hash func coeffs
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
    return sizeof(*this) + (t * k + 3) * sizeof(uint64_t);
    // + this->seen.size()
}

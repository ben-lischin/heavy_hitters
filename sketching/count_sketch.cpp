#include "sketch.hpp"
#include <algorithm>

CountSketch::CountSketch(uint64_t t, uint64_t k) : m(0), t(t), k(k) {
    this->table = (int64_t*) malloc(t * k * sizeof(int64_t));
}

CountSketch::~CountSketch() {
    free(this->table);
    this->table = nullptr;
}


inline uint64_t CountSketch::BucketHash(uint64_t x, uint64_t row) {
    return MurmurHash64A(&x, sizeof(x), row) % this->k;
}

inline int8_t CountSketch::UpdateHash(uint64_t x, uint64_t row) {
    uint64_t hash = MurmurHash64A(&x, sizeof(x), row + 0xBEEF);
    return (hash & 1) ? 1 : -1;
}

void CountSketch::Add(uint64_t x) {
    for (uint64_t row = 0; row < this->t; row++) {
        uint64_t bucket = BucketHash(x, row);
        int8_t update = UpdateHash(x, row);
        table[row * this->k + bucket] += update;
    }

    this->m++;

    this->seen.insert(x);
}

uint64_t CountSketch::Estimate(uint64_t x) {
    std::vector<int64_t> counters;
    counters.reserve(t);

    for (uint64_t row = 0; row < t; row++) {
        uint64_t bucket = BucketHash(x, row);
        int8_t sign = UpdateHash(x, row);
        int64_t count = sign * table[row * this->k + bucket];
        counters.push_back(count);
    }

    // median
    std::nth_element(counters.begin(), counters.begin() + t / 2, counters.end());
    return std::max(counters[t / 2], int64_t(0)); // no negative counts
}

std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> CountSketch::HeavyHitters(double phi) {
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

size_t CountSketch::Size() {
    return sizeof(*this) + (t * k + 3) * sizeof(uint64_t);
    // + this->seen.size()
}
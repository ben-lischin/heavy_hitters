#include "sketch.hpp"
#include <algorithm>

CountSketch::CountSketch(uint64_t t, uint64_t k) : m(0), t(t), k(k) {
    // k must be power of 2 for efficient hashing techniques
    assert((k & (k - 1)) == 0 && k > 0);

    this->table = (int64_t*) malloc(t * k * sizeof(int64_t));
    this->hash_coeffs = (uint64_t*) malloc(t * 4 * sizeof(uint64_t));

    // coefficients for t pairwise independent hash functions

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> distrib_a(1ULL, LARGE_PRIME - 1ULL); // 0 < a < p
    std::uniform_int_distribution<uint64_t> distrib_b(0ULL, LARGE_PRIME - 1ULL); // 0 ≤ b < p

    for (uint64_t i = 0; i < 4*t; i+=4) {
        hash_coeffs[i] = distrib_a(gen);
        hash_coeffs[i + 1] = distrib_b(gen);
        hash_coeffs[i + 2] = distrib_a(gen);
        hash_coeffs[i + 3] = distrib_b(gen);
    }

    // srand(time(NULL));
    // for (uint64_t i = 0; i < t * 4; i+=4) {
    //     // hash_coeffs[i] = uint64_t(float(rand())*float(LARGE_PRIME)/float(RAND_MAX) + 1); // a1
    //     // hash_coeffs[i + 1] = uint64_t(float(rand())*float(LARGE_PRIME)/float(RAND_MAX) + 1); // b1
    //     // hash_coeffs[i + 2] = uint64_t(float(rand())*float(LARGE_PRIME)/float(RAND_MAX) + 1); // a2
    //     // hash_coeffs[i + 3] = uint64_t(float(rand())*float(LARGE_PRIME)/float(RAND_MAX) + 1); // b2
    //     hash_coeffs[i] = uint64_t(rand()) + 1; // 0 < a1 < p
    //     hash_coeffs[i + 1] = uint64_t(rand()); // 0 ≤ b1 < p
    //     hash_coeffs[i + 2] = uint64_t(rand()) + 1; // 0 < a2 < p
    //     hash_coeffs[i + 3] = uint64_t(rand()); // 0 ≤ b2 < p
    // }
}

CountSketch::~CountSketch() {
    free(this->table);
    this->table = nullptr;

    free(this->hash_coeffs);
    this->hash_coeffs = nullptr;
}

inline uint64_t CountSketch::BucketHash(uint64_t x, uint64_t row) {
    uint64_t a1 = this->hash_coeffs[row * 4];
    uint64_t b1 = this->hash_coeffs[row * 4 + 1];

    // cast up to avoid overflow
    __uint128_t u = (__uint128_t)a1 * x + b1;

    // return (uint64_t)(u % LARGE_PRIME) % this->k;

    // optimization...

    // low order bits: u % 2^89
    __uint128_t lo = u & LARGE_PRIME;
    // high order bits: u / 2^89
    __uint128_t hi = u >> 89;
   
    // hi + lo < LARGE_PRIME, and (hi + lo) ≡ u (mod LARGE_PRIME)
    __uint128_t hash = hi + lo;

    // map hash to a counter bucket
    hash = hash & (this->k - 1); // hash % k, assuming k is a power of 2

    return (uint64_t)hash;
}

inline int8_t CountSketch::UpdateHash(uint64_t x, uint64_t row) {
    uint64_t a2 = this->hash_coeffs[row * 4 + 2];
    uint64_t b2 = this->hash_coeffs[row * 4 + 3];

    // cast up to avoid overflow
    __uint128_t u = (__uint128_t)a2 * x + b2;

    // uint64_t hash = (uint64_t)(u % LARGE_PRIME);
    // return (hash & 1UL) ? 1 : -1;

    // optimization...

    // low order bits: u % 2^89
    __uint128_t lo = u & LARGE_PRIME;
    // high order bits: u / 2^89
    __uint128_t hi = u >> 89;
  
    // hi + lo < LARGE_PRIME, and (hi + lo) ≡ u (mod LARGE_PRIME)
    __uint128_t hash = hi + lo;

    return (hash & 1ULL) * 2 - 1; // 1 or -1 depending on hash parity
}

void CountSketch::Add(uint64_t x) {
    for (uint64_t row = 0; row < this->t; row++) {
        uint64_t bucket = BucketHash(x, row);
        int8_t update = UpdateHash(x, row);
        table[row * this->k + bucket] += update;
    }

    this->m++;

    // working heavy hitter candidates
    if (this->Estimate(x) >= this->m * MIN_PHI) {
        this->seen.insert(x);
    }
}

uint64_t CountSketch::Estimate(uint64_t x) {
    std::vector<int64_t> counters = std::vector<int64_t>(t);

    for (uint64_t row = 0; row < t; row++) {
        uint64_t bucket = BucketHash(x, row);
        int8_t sign = UpdateHash(x, row);
        int64_t count = sign * table[row * this->k + bucket];
        counters[row] = count;
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
    return sizeof(*this) + (3 + (this->t * this->k) + (this->k * 4) + this->seen.size()) * sizeof(uint64_t);
}
#ifndef SKETCH_H
#define SKETCH_H

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cstdint>
#include <random>
#include "../hashutil.h"


const uint64_t LARGE_PRIME = 0xFFFFFFFFFFFFFFC5;

class Sketch {
    public:
        // increments the count of item x by 1
        virtual void Add(uint64_t x) = 0;
        // returns the estimated frequency of item x
        virtual uint64_t Estimate(uint64_t x) = 0;
        // calculates the phi-heavy hitters with frequency > phi*N
        virtual std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) = 0;
        // return the size of the sketch (allocated memory)
        virtual size_t Size() = 0;
    private:
        // total count of items seen
        uint64_t m;
};

class MisraGries : public Sketch {
    public:
        MisraGries(uint64_t capacity);
        void Add(uint64_t x) override;
        uint64_t Estimate(uint64_t x) override;
        std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        size_t Size() override;
    private:
        // total count of items seen
        uint64_t m;
        // capacity
        uint64_t k;
        // key : count for up to top k items
        std::unordered_map<uint64_t, uint64_t> counters;

        
};

class CountSketch : public Sketch {
    public:
        // t = num hash functions, k = num counters per hash func 
        CountSketch(uint64_t t, uint64_t k);
        ~CountSketch();
        void Add(uint64_t x) override;
        uint64_t Estimate(uint64_t x) override;
        std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        size_t Size() override;
    private:
        // total count of items seen
        uint64_t m;
        // rows ~ num hash funcs
        uint64_t t;
        // cols ~ num counter buckets
        uint64_t k;
        // counting table
        int64_t *table;

        // hash coefficients {a1, b1, a2, b2}[]
        uint64_t *hash_coeffs;

        // candidates for heavy hitters
        std::unordered_set<uint64_t> seen;

        // first hash function for assigning a counter to update in the row
        inline uint64_t BucketHash(uint64_t x, uint64_t row);
        // second hash function for incrementing or decrementing the counter
        inline int8_t UpdateHash(uint64_t x, uint64_t row);

        
};

class CountMinSketch : public Sketch {
    public:
        // t = num hash functions, k = num counters (buckets per row)
        CountMinSketch(uint64_t t, uint64_t k);
        ~CountMinSketch();
        void Add(uint64_t x) override;
        uint64_t Estimate(uint64_t x) override;
        std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        size_t Size() override;
    private:
        // total count of items seen
        uint64_t m;
        // table rows ~ num hash funcs
        uint64_t t;
        // table cols ~ num counter buckets
        uint64_t k;
        // counting table
        uint64_t *table;

        // hash coefficients {a, b}[]
        uint64_t *hash_coeffs;

        // candidates for heavy hitters
        std::unordered_set<uint64_t> seen;

        // hash function for assigning a counter to update in the row
        inline uint64_t BucketHash(uint64_t x, uint64_t row);

};

#endif
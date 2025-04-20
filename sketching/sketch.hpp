#ifndef SKETCH_H
#define SKETCH_H

#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>
#include "../hashutil.h"

class Sketch {
    public:
        // increments the count of item x by 1
        virtual void Add(uint64_t x) = 0;
        // returns the estimated frequency of item x
        virtual uint64_t Estimate(uint64_t x) = 0;
        // // calculates the phi-heavy hitters
        // // returns a set of items where the count of each item in the set is bigger than phi*N, where N is the size of the stream
        // virtual std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) = 0;
        // // return the size of the sketch (allocated memory)
        // virtual size_t Size() = 0;
    private:
        // total count of items seen
        uint64_t m;
};

class MisraGries : public Sketch {
    public:
        MisraGries(uint64_t capacity);
        void Add(uint64_t x) override;
        uint64_t Estimate(uint64_t x) override;
        std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi);
        // std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        size_t Size();
        // size_t Size() override;
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
        // std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        // size_t Size() override;
    private:
        // total count of items seen
        uint64_t m;
        // rows ~ num hash funcs
        uint64_t t;
        // cols ~ num counter buckets
        uint64_t k;
        // counting table
        int64_t *table;

        // first hash function for assigning a counter to update in the row
        inline uint64_t BucketHash(uint64_t x, uint64_t row);
        // second hash function for incrementing or decrementing the counter
        inline int8_t UpdateHash(uint64_t x, uint64_t row);

        
};

class CountMinSketch : public Sketch {
    public:
        // t = num hash functions, k = num counters (buckets per row)
        // for calculating heavy hitters, we should use an expected stream size N and phi for allocating a minheap
        CountMinSketch(uint64_t t, uint64_t k);
        ~CountMinSketch();
        void Add(uint64_t x) override;
        uint64_t Estimate(uint64_t x) override;
        // std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> HeavyHitters(double phi) override;
        // size_t Size() override;
    private:
        // total count of items seen
        uint64_t m;
        // table rows ~ num hash funcs
        uint64_t t;
        // table cols ~ num counter buckets
        uint64_t k;
        // counting table
        uint64_t *table;

        // hash function for assigning a counter to update in the row
        inline uint64_t BucketHash(uint64_t x, uint64_t row);


        // // minheap for maintaining heavy hitters
        // std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> minHeap


};

#endif
// Author: Prashant Pandey <prashant.pandey@utah.edu>
// For use in CS6968 & CS5968

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <openssl/rand.h>
#include <unordered_map>

#include "sketching/sketch.hpp"
#include "zipf.h"

using namespace std::chrono;

#define UNIVERSE 1ULL << 30
#define EXP 1.5

double elapsed(high_resolution_clock::time_point t1,
            high_resolution_clock::time_point t2) {
    return (duration_cast<duration<double>>(t2 - t1)).count();
}

std::pair<double, double> compute_precision_recall(
    const std::multimap<uint64_t, uint64_t, std::greater<uint64_t>>& truth_hh,
    const std::multimap<uint64_t, uint64_t, std::greater<uint64_t>>& sketch_hh
) {
    auto it1 = truth_hh.begin();
    auto it2 = sketch_hh.begin();

    // true positive, false positive, false negative
    float tp = 0, fp = 0, fn = 0;

    while (it1 != truth_hh.end() && it2 != sketch_hh.end()) {
        if (it1->second == it2->second) {
            tp++;
            ++it1;
            ++it2;
        } else if (it1->first > it2->first) {
            fn++;
            ++it1;
        } else if (it1->first < it2->first) {
            fp++;
            ++it2;
        } else {
            if (sketch_hh.find(it1->second) != sketch_hh.end()) {
                tp++;
            } else {
                fn++;
            }
            ++it1;
        }
    }

    double precision = tp + fp > 0 ? tp / (tp + fp) : 0.0;
    double recall = tp + fn > 0 ? tp / (tp + fn) : 0.0;

    return {precision, recall};
}

int main(int argc, char **argv) {
    // Setup arguments and generate random numbers 
    if (argc < 3) {
        std::cerr << "Specify the number of items N and phi.\n";
        exit(1);
    }
    uint64_t N = atoi(argv[1]);
    double phi = atof(argv[2]);
    uint64_t *numbers = (uint64_t *)malloc(N * sizeof(uint64_t));
    if (!numbers) {
        std::cerr << "Malloc numbers failed.\n";
        exit(0);
    }
    high_resolution_clock::time_point t1, t2;
    t1 = high_resolution_clock::now();
    generate_random_keys(numbers, UNIVERSE, N, EXP);
    t2 = high_resolution_clock::now();
    std::cout << "Time to generate " << N << " items: " << elapsed(t1, t2) << " secs\n\n";


    // ------------- DATA STRUCTURES -------------

    std::unordered_map<uint64_t, uint64_t> map(N);
    CountSketch cs(5, 10000);
    CountMinSketch cms(5, 1000);
    MisraGries mg(10);


    // ------------- COUNTING -------------

    // Hash Table          
    t1 = high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        map[numbers[i]]++;
    }
    t2 = high_resolution_clock::now();
    std::cout << "Time to count " << N << " items with Hash Table: " << elapsed(t1, t2) << " secs\n";

    // Count Sketch
    t1 = high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        cs.Add(numbers[i]);
    }
    t2 = high_resolution_clock::now();
    std::cout << "Time to count " << N << " items with Count Sketch: " << elapsed(t1, t2) << " secs\n";

    // Count-Min Sketch
    t1 = high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        cms.Add(numbers[i]);
    }
    t2 = high_resolution_clock::now();
    std::cout << "Time to count " << N << " items with Count-Min Sketch: " << elapsed(t1, t2) << " secs\n";

    // Misra-Gries
    t1 = high_resolution_clock::now();
    for (uint64_t i = 0; i < N; ++i) {
        mg.Add(numbers[i]);
    }
    t2 = high_resolution_clock::now();
    std::cout << "Time to count " << N << " items with Misra-Gries: " << elapsed(t1, t2) << " secs\n\n";

    // free stream after single pass
    free(numbers);


    // ------------- Heavy Hitters -------------

    // Hash Table
    uint64_t total = 0;
    double threshold = phi * N;
    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> ht_hh;
    t1 = high_resolution_clock::now();
    for (auto it = map.begin(); it != map.end(); ++it) {
    if (it->second >= threshold) {
         ht_hh.insert(std::make_pair(it->second, it->first));
    }
    total += it->second;
    }
    t2 = high_resolution_clock::now();
    std::cout << "Hash Table time to compute phi-heavy hitters: " << elapsed(t1, t2) << " secs\n";
    assert(total == N);

    // Count Sketch
    t1 = high_resolution_clock::now();
    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> cs_hh = cs.HeavyHitters(phi);
    t2 = high_resolution_clock::now();
    std::cout << "Count Sketch time to compute phi-heavy hitters: " << elapsed(t1, t2) << " secs\n";

    // Count-Min Sketch
    t1 = high_resolution_clock::now();
    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> cms_hh = cms.HeavyHitters(phi);
    t2 = high_resolution_clock::now();
    std::cout << "Count-Min Sketch time to compute phi-heavy hitters: " << elapsed(t1, t2) << " secs\n";

    // Misra-Gries
    t1 = high_resolution_clock::now();
    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> mg_hh = mg.HeavyHitters(phi);
    t2 = high_resolution_clock::now();
    std::cout << "Misra-Gries time to compute phi-heavy hitters: " << elapsed(t1, t2) << " secs\n\n";

    
    // ------------- Precision & Recall -------------

    auto cs_precision_recall = compute_precision_recall(ht_hh, cs_hh);
    std::cout << "Count Sketch { Precision, Recall } : { " << cs_precision_recall.first << ", " << cs_precision_recall.second << " }\n";
    auto cms_precision_recall = compute_precision_recall(ht_hh, cms_hh);
    std::cout << "Count-Min Sketch { Precision, Recall } : { " << cms_precision_recall.first << ", " << cms_precision_recall.second << " }\n";
    auto mg_precision_recall = compute_precision_recall(ht_hh, mg_hh);
    std::cout << "Misra-Gries { Precision, Recall } : { " << mg_precision_recall.first << ", " << mg_precision_recall.second  << " }\n\n";


    // ------------- Memory Usage -------------

	uint64_t ht_size = map.size() * (sizeof(uint64_t) * 2);
    std::cout << "Hash Table size: " << ht_size << " bytes\n";
    std::cout << "Count Sketch size: " << cs.Size() << " bytes (saved : " << ht_size - cs.Size() << " bytes)\n";
    std::cout << "Count-Min Sketch size: " << cms.Size() << " bytes (saved : " << ht_size - cms.Size() << " bytes)\n";
    std::cout << "Misra-Gries size: " << mg.Size() << " bytes (saved : " << ht_size - mg.Size() << " bytes)\n";

    return 0;
}

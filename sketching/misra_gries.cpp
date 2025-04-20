#include "sketch.hpp"

MisraGries::MisraGries(uint64_t k) : k(k) {}

void MisraGries::Add(uint64_t x) {
    if (this->counters.count(x)) {
        this->counters[x]++;
    } else if (this->counters.size() < this->k - 1) {
        this->counters[x] = 1;
    } else {
        for (auto it = this->counters.begin(); it != this->counters.end(); ) {
            if (--(it->second) == 0) {
                it = this->counters.erase(it);
            } else {
                ++it;
            }
        }
    }

    this->m++;
}

uint64_t MisraGries::Estimate(uint64_t x) {
    auto it = counters.find(x);
    return it != counters.end() ? it->second : 0;
}

std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> MisraGries::HeavyHitters(double phi) {
    uint64_t threshold = phi * this->m;

    std::multimap<uint64_t, uint64_t, std::greater<uint64_t>> hh;
    for (const auto& [key, count] : this->counters) {
        if (count > threshold) {
            hh.insert({count, key});
        }
    }

    return hh;
}

size_t MisraGries::Size() {
    return sizeof(*this) + this->counters.size() * (sizeof(uint64_t) * 2);
}

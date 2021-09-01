#include "Utils/Assert.hpp"

#include "gtest/gtest.h"

#include <cstdint>


using Empty = ::testing::Test;


uint64_t Forrest_G (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t m)
{
    uint64_t G = seed % m;
    uint64_t h = g;

    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            G = (G * h) % m;
        }
        h = (h * h) % m;
        i = i / 2;
    }

    return G;
}


uint64_t Forrest_C (const uint64_t k, const uint64_t seed, const uint64_t g, const uint64_t c, const uint64_t m)
{
    uint64_t C = seed % m;
    uint64_t f = c;
    uint64_t h = g;
    uint64_t i = (k + m) % m;

    while (i > 0) {
        if (i % 2 == 1) {
            C = (C * h + f) % m;
        }
        f = (f * (h + 1)) % m;
        h = (h * h) % m;
        i = i / 2;
    }

    return C;
}


TEST_F (Empty, ArbitraryStrideLCG)
{
    double   sum     = 0.0;
    int      count   = 0;
    uint64_t modolus = 2147483647; //(static_cast<uint64_t> (1) << 31) - 1; // 2^31 - 1
    uint64_t mul_a   = 48271;      // minstd_rand
    uint64_t inc_c   = 0;

    for (int i = 10; i < 100000; ++i) {
        double val  = static_cast<double> (Forrest_G (i, 634, mul_a, modolus));
        double val2 = static_cast<double> (Forrest_C (i, 634, mul_a, 0, modolus));
        GVK_ASSERT (val == val2);
        const double perc = val / modolus;
        sum += perc;
        ++count;
    }

    std::cout << sum / count << std::endl;
}

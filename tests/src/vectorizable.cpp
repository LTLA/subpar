#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "subpar/vectorizable.hpp"

TEST(Vectorizable, Basic) {
    std::vector<double> x(100), y(x.size());

    std::mt19937_64 rng;
    std::normal_distribution<> dist;
    for (auto& xi : x) {
        xi = dist(rng);
    }
    std::iota(y.begin(), y.end(), 0);

    std::vector<double> ref(x.size());
    for (size_t i = 0, end = ref.size(); i < end; ++i) {
        ref[i] = x[i] + y[i];
    }

    std::vector<double> alt(x.size());
    SUBPAR_VECTORIZABLE
    for (size_t i = 0, end = ref.size(); i < end; ++i) {
        alt[i] = x[i] + y[i];
    }

    EXPECT_EQ(ref, alt);
}

TEST(Vectorizable, Indexed) {
    std::vector<double> x(100);
    std::mt19937_64 rng;
    std::normal_distribution<> dist;
    for (auto& xi : x) {
        xi = dist(rng);
    }

    std::vector<int> y;
    for (int i = 0; i < 10; ++i) {
        y.push_back(i * 10);
    }

    std::vector<double> ref(y.size());
    std::iota(ref.begin(), ref.end(), -10.0);
    for (size_t i = 0, end = ref.size(); i < end; ++i) {
        ref[i] += x[y[i]];
    }

    std::vector<double> alt(y.size());
    std::iota(alt.begin(), alt.end(), -10.0);
    SUBPAR_VECTORIZABLE
    for (size_t i = 0, end = ref.size(); i < end; ++i) {
        alt[i] += x[y[i]];
    }

    EXPECT_EQ(ref, alt);
}

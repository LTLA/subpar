#include <gtest/gtest.h>

#include <vector>
#include <cstdint>

#define SUBPAR_CUSTOM_PARALLELIZE_RANGE(nw, nt, run) ::subpar::test_parallelize_range(nw, nt, std::move(run))
#include "subpar/subpar.hpp"

TEST(ParallelizeRange, ExactPartition) {
    std::vector<int> thread_counts { 2, 4, 5, 10 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        subpar::parallelize_range(tn, assignments.size(), [&](int t, int start, int len) {
            std::fill_n(assignments.begin() + start, len, t);
        });

        for (auto a : assignments) {
            EXPECT_LT(a, tn);
        }
    }
}

TEST(ParallelizeRange, InexactPartition) {
    std::vector<int> thread_counts { 3, 6, 7, 9, 11 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        subpar::parallelize_range(tn, assignments.size(), [&](int t, int start, int len) {
            std::fill_n(assignments.begin() + start, len, t);
        });

        for (auto a : assignments) {
            EXPECT_LT(a, tn);
        }
    }
}

TEST(ParallelizeRange, Overprovision) {
    int tn = 10;
    std::vector<int> assignments(5, /* dummy value */ 255);
    subpar::parallelize_range(tn, assignments.size(), [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    for (auto a : assignments) {
        EXPECT_LT(a, tn);
    }
}

TEST(ParallelizeRange, NoThread) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize_range(/* nthreads = */ 0, /* ntasks = */ assignments.size(), [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    for (auto a : assignments) {
        EXPECT_EQ(a, 0);
    }
}

TEST(ParallelizeRange, NoTasks) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize_range(/* nthreads = */ 10, /* ntasks = */ 0, [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    for (auto a : assignments) {
        EXPECT_EQ(a, 255);
    }
}

TEST(ParallelizeRange, NoIntervals) {
    int tn = 10;
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::test_parallelize_range(tn, assignments.size(), [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    }, /* scaling = */ -1.0);

    EXPECT_LT(assignments.front(), tn);
    for (auto a : assignments) {
        EXPECT_EQ(a, assignments.front());
    }
}

TEST(ParallelizeRange, SmallIntegers) {
    // This test checks that the range calculations do not overflow. 
    int tn = 10;
    uint8_t njobs = -1;
    std::vector<int> assignments(njobs, /* dummy value */ 255);
    subpar::parallelize_range(/* nthreads = */ tn, njobs, [&](int t, uint8_t start, uint8_t len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    for (auto a : assignments) {
        EXPECT_LT(a, tn);
    }
}

#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

#ifdef CUSTOM_PARALLEL_TEST
template<typename Task_, typename Run_>
void stupid_parallel_simple(Task_ num_tasks, Run_ run) {
    for (Task_ t = 0; t < num_tasks; ++t) {
        run(t);
    }
}

#define SUBPAR_CUSTOM_PARALLELIZE_SIMPLE stupid_parallel_simple
#ifdef CUSTOM_PARALLEL_TEST_NOTHROW
#define SUBPAR_CUSTOM_PARALLELIZE_SIMPLE_NOTHROW stupid_parallel_simple
#endif
#endif

#include "subpar/simple.hpp"

TEST(ParallelizeSimple, UsesOmp) {
#ifdef SUBPAR_USES_OPENMP
    bool uses_openmp = SUBPAR_USES_OPENMP_SIMPLE;
#else
    bool uses_openmp = false;
#endif

#if defined(SUBPAR_CUSTOM_PARALLELIZE_SIMPLE) || !defined(_OPENMP)
    EXPECT_FALSE(uses_openmp);
#else
    EXPECT_TRUE(uses_openmp);
#endif
}

TEST(ParallelizeSimple, Basic) {
    std::vector<int> thread_counts { 0, 1, 3, 6, 7, 9, 11 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(tn, /* dummy value */ 255);
        subpar::parallelize_simple(tn, [&](int t) { assignments[t] = 1; });
        std::vector<int> expected(assignments.size(), 1);
        EXPECT_EQ(assignments, expected);
    }
}

TEST(ParallelizeSimple, Errors) {
    EXPECT_ANY_THROW({
        try {
            subpar::parallelize_simple(2, [&](int) -> void {
                throw std::runtime_error("WHEE");
            });
        } catch (std::exception& e) {
            EXPECT_TRUE(std::string(e.what()).find("WHEE") != std::string::npos);
            throw;
        } catch (...) {
            // don't do anything, it shouldn't get here.
        }
    });

    EXPECT_ANY_THROW({
        subpar::parallelize_simple(2, [&](int) -> void {
            throw 1;
        });
    });
}

TEST(ParallelizeSimple, Nothrow) {
    std::vector<int> assignments(5, /* dummy value */ 255);
    subpar::parallelize_simple<true>(assignments.size(), [&](int t) { assignments[t] = 1; });
    std::vector<int> expected(assignments.size(), 1);
    EXPECT_EQ(assignments, expected);
}

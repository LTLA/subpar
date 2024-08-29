#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

#ifdef CUSTOM_PARALLEL_TEST
template<typename Task_, typename Run_>
void stupid_parallel(int num_threads, Task_ num_tasks, Run_ run) {
    if (num_threads == 0) {
        return;
    }

    std::vector<Task_> allocations(num_threads);
    for (Task_ t = 0; t < num_tasks; ++t) {
        ++(allocations[t % num_threads]);
    }

    Task_ start = 0; 
    for (int t = 0; t < num_threads; ++t) {
        if (allocations[t]) {
            run(t, start, allocations[t]);
            start += allocations[t];
        }
    }
}

#define SUBPAR_CUSTOM_PARALLELIZE_RANGE stupid_parallel
#ifdef CUSTOM_PARALLEL_TEST_NOTHROW
#define SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW stupid_parallel
#endif
#endif

#include "subpar/subpar.hpp"

static std::vector<int> tabulate(const std::vector<int>& assignments) {
    size_t maxed = *std::max_element(assignments.begin(), assignments.end()) + 1;
    std::vector<int> count(maxed);
    for (auto a : assignments) {
        ++count[a];
    }
    return count;
}

static void check_ranges(const std::vector<std::pair<int, int> >& ranges, int num_tasks) {
    ASSERT_EQ(ranges.front().first, 0);
    size_t last = ranges.front().second;

    for (size_t i = 1; i < ranges.size(); ++i) {
        EXPECT_EQ(last, ranges[i].first);
        last = ranges[i].first + ranges[i].second;
    }

    EXPECT_EQ(last, num_tasks);
}

TEST(ParallelizeRange, UsesOmp) {
#ifdef SUBPAR_USES_OPENMP
    bool uses_openmp = SUBPAR_USES_OPENMP;
#else
    bool uses_openmp = false;
#endif

#if defined(SUBPAR_CUSTOM_PARALLEL) || !defined(_OPENMP)
    EXPECT_FALSE(uses_openmp);
#else
    EXPECT_TRUE(uses_openmp);
#endif
}

TEST(ParallelizeRange, ExactPartition) {
    std::vector<int> thread_counts { 2, 4, 5, 10 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        std::vector<std::pair<int, int> > ranges(tn);
        subpar::parallelize_range(ranges.size(), assignments.size(), [&](int t, int start, int len) {
            ranges[t].first = start;
            ranges[t].second = len;
            std::fill_n(assignments.begin() + start, len, t);
        });

        auto tabled = tabulate(assignments);
        std::vector<int> exact(tn, assignments.size() / tn);
        EXPECT_EQ(tabled, exact);
        check_ranges(ranges, assignments.size());
    }
}

TEST(ParallelizeRange, InexactPartition) {
    std::vector<int> thread_counts { 3, 6, 7, 9, 11 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        std::vector<std::pair<int, int> > ranges(tn);
        subpar::parallelize_range(tn, assignments.size(), [&](int t, int start, int len) {
            ranges[t].first = start;
            ranges[t].second = len;
            std::fill_n(assignments.begin() + start, len, t);
        });

        auto tabled = tabulate(assignments);
        EXPECT_EQ(tabled.size(), tn);
        auto lower = assignments.size() / tn;
        EXPECT_EQ(*std::min_element(tabled.begin(), tabled.end()), lower);
        EXPECT_EQ(*std::max_element(tabled.begin(), tabled.end()), lower + 1);
        check_ranges(ranges, assignments.size());
    }
}

TEST(ParallelizeRange, Overprovision) {
    std::vector<int> assignments(5, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(7);
    subpar::parallelize_range(ranges.size(), assignments.size(), [&](int t, int start, int len) {
        ranges[t].first = start;
        ranges[t].second = len;
        std::fill_n(assignments.begin() + start, len, t);
    });

    auto tabled = tabulate(assignments);
    std::vector<int> expected(assignments.size(), 1);
    EXPECT_EQ(tabled, expected);

    for (size_t i = assignments.size(); i < ranges.size(); ++i) {
        EXPECT_EQ(ranges[i], std::make_pair(0, 0));
    }
    ranges.resize(assignments.size());
    check_ranges(ranges, assignments.size());
}

TEST(ParallelizeRange, OneThread) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(1);
    subpar::parallelize_range(ranges.size(), assignments.size(), [&](int t, int start, int len) {
        ranges[t].first = start;
        ranges[t].second = len;
        std::fill_n(assignments.begin() + start, len, t);
    });

    auto tabled = tabulate(assignments);
    EXPECT_EQ(tabled.size(), 1);
    EXPECT_EQ(tabled.front(), assignments.size());
    check_ranges(ranges, assignments.size());
}

TEST(ParallelizeRange, OneTask) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize_range(/* nthreads = */ 10, /* ntask = */ 1, [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    EXPECT_EQ(assignments.front(), 0);
}

TEST(ParallelizeRange, NoTasks) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize_range(/* nthreads = */ 10, /* ntasks = */ 0, [&](int t, int start, int len) {
        std::fill_n(assignments.begin() + start, len, t);
    });

    EXPECT_EQ(assignments.front(), 255);
    EXPECT_EQ(assignments.back(), 255);
}

TEST(ParallelizeRange, SmallIntegers) {
    // This test checks that the range calculations do not overflow. 
    uint8_t njobs = -1;
    std::vector<int> assignments(njobs, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(10);
    subpar::parallelize_range(/* nthreads = */ 10, njobs, [&](int t, uint8_t start, uint8_t len) {
        ranges[t].first = start;
        ranges[t].second = len;
        std::fill_n(assignments.begin() + start, len, t);
    });

    auto tabled = tabulate(assignments);
    EXPECT_EQ(tabled.size(), ranges.size());
    auto lower = assignments.size() / ranges.size();
    EXPECT_EQ(*std::min_element(tabled.begin(), tabled.end()), lower);
    EXPECT_EQ(*std::max_element(tabled.begin(), tabled.end()), lower + 1);
    check_ranges(ranges, assignments.size());
}

TEST(ParallelizeRange, Errors) {
    EXPECT_ANY_THROW({
        try {
            subpar::parallelize_range(255, 2, [&](size_t, int, int) -> void {
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
        subpar::parallelize_range(255, 2, [&](size_t, int, int) -> void {
            throw 1;
        });
    });
}

TEST(ParallelizeRange, Nothrow) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    int tn = 5;

    std::vector<std::pair<int, int> > ranges(tn);
    subpar::parallelize_range<true>(ranges.size(), assignments.size(), [&](int t, int start, int len) {
        ranges[t].first = start;
        ranges[t].second = len;
        std::fill_n(assignments.begin() + start, len, t);
    });

    auto tabled = tabulate(assignments);
    std::vector<int> exact(tn, assignments.size() / tn);
    EXPECT_EQ(tabled, exact);
    check_ranges(ranges, assignments.size());
}

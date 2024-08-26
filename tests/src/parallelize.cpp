#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

#ifdef CUSTOM_PARALLEL_TEST
template<typename Task_, typename Setup_, typename Run_>
void stupid_parallel(int num_threads, Task_ num_tasks, Setup_ setup, Run_ run) {
    if (num_threads == 0) {
        return;
    }

    auto wrk = setup();
    std::vector<Task_> allocations(num_threads);
    for (Task_ t = 0; t < num_tasks; ++t) {
        ++(allocations[t % num_threads]);
    }

    Task_ start = 0; 
    for (int t = 0; t < num_threads; ++t) {
        if (allocations[t]) {
            run(t, start, allocations[t], wrk);
            start += allocations[t];
        }
    }
}

#define SUBPAR_CUSTOM_PARALLEL stupid_parallel
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

TEST(Parallelize, ExactPartition) {
    std::vector<int> thread_counts { 2, 4, 5, 10 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        std::vector<std::pair<int, int> > ranges(tn);
        subpar::parallelize(
            ranges.size(),
            assignments.size(),
            [&]() -> bool {
                return true;
            }, 
            [&](int t, int start, int len, bool&) {
                ranges[t].first = start;
                ranges[t].second = len;
                std::fill_n(assignments.begin() + start, len, t);
            }
        );

        auto tabled = tabulate(assignments);
        std::vector<int> exact(tn, assignments.size() / tn);
        EXPECT_EQ(tabled, exact);
        check_ranges(ranges, assignments.size());
    }
}

TEST(Parallelize, InexactPartition) {
    std::vector<int> thread_counts { 3, 6, 7, 9, 11 };

    for (auto tn : thread_counts) {
        std::vector<int> assignments(1000, /* dummy value */ 255);
        std::vector<std::pair<int, int> > ranges(tn);
        subpar::parallelize(
            tn,
            assignments.size(),
            [&]() -> bool {
                return true;
            }, 
            [&](int t, int start, int len, bool&) {
                ranges[t].first = start;
                ranges[t].second = len;
                std::fill_n(assignments.begin() + start, len, t);
            }
        );

        auto tabled = tabulate(assignments);
        EXPECT_EQ(tabled.size(), tn);
        auto lower = assignments.size() / tn;
        EXPECT_EQ(*std::min_element(tabled.begin(), tabled.end()), lower);
        EXPECT_EQ(*std::max_element(tabled.begin(), tabled.end()), lower + 1);
        check_ranges(ranges, assignments.size());
    }
}

TEST(Parallelize, Overprovision) {
    std::vector<int> assignments(5, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(7);
    subpar::parallelize(
        ranges.size(),
        assignments.size(),
        [&]() -> bool {
            return true;
        }, 
        [&](int t, int start, int len, bool&) {
            ranges[t].first = start;
            ranges[t].second = len;
            std::fill_n(assignments.begin() + start, len, t);
        }
    );

    auto tabled = tabulate(assignments);
    std::vector<int> expected(assignments.size(), 1);
    EXPECT_EQ(tabled, expected);

    for (size_t i = assignments.size(); i < ranges.size(); ++i) {
        EXPECT_EQ(ranges[i], std::make_pair(0, 0));
    }
    ranges.resize(assignments.size());
    check_ranges(ranges, assignments.size());
}

TEST(Parallelize, OneThread) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(1);
    subpar::parallelize(
        ranges.size(),
        assignments.size(),
        [&]() -> bool {
            return true;
        }, 
        [&](int t, int start, int len, bool&) {
            ranges[t].first = start;
            ranges[t].second = len;
            std::fill_n(assignments.begin() + start, len, t);
        }
    );

    auto tabled = tabulate(assignments);
    EXPECT_EQ(tabled.size(), 1);
    EXPECT_EQ(tabled.front(), assignments.size());
    check_ranges(ranges, assignments.size());
}

TEST(Parallelize, OneTask) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize(
        10,
        1,
        [&]() -> bool {
            return true;
        }, 
        [&](int t, int start, int len, bool&) {
            std::fill_n(assignments.begin() + start, len, t);
        }
    );

    EXPECT_EQ(assignments.front(), 0);
}

TEST(Parallelize, NoTasks) {
    std::vector<int> assignments(1000, /* dummy value */ 255);
    subpar::parallelize(
        10,
        0,
        [&]() -> bool {
            return true;
        }, 
        [&](int t, int start, int len, bool&) {
            std::fill_n(assignments.begin() + start, len, t);
        }
    );

    EXPECT_EQ(assignments.front(), 255);
    EXPECT_EQ(assignments.back(), 255);
}

TEST(Parallelize, SmallIntegers) {
    uint8_t njobs = -1;
    std::vector<int> assignments(njobs, /* dummy value */ 255);
    std::vector<std::pair<int, int> > ranges(10);
    subpar::parallelize(
        10,
        njobs,
        [&]() -> bool {
            return true;
        }, 
        [&](int t, uint8_t start, uint8_t len, bool&) {
            ranges[t].first = start;
            ranges[t].second = len;
            std::fill_n(assignments.begin() + start, len, t);
        }
    );

    auto tabled = tabulate(assignments);
    EXPECT_EQ(tabled.size(), ranges.size());
    auto lower = assignments.size() / ranges.size();
    EXPECT_EQ(*std::min_element(tabled.begin(), tabled.end()), lower);
    EXPECT_EQ(*std::max_element(tabled.begin(), tabled.end()), lower + 1);
    check_ranges(ranges, assignments.size());
}

TEST(Parallelize, Errors) {
    EXPECT_ANY_THROW({
        try {
            subpar::parallelize(
                255,
                2,
                []() -> bool {
                    return true;
                },
                [&](size_t, int, int, bool&) -> void {
                    throw std::runtime_error("WHEE");
                }
            );
        } catch (std::exception& e) {
            EXPECT_TRUE(std::string(e.what()).find("WHEE") != std::string::npos);
            throw;
        } catch (...) {
            // don't do anything, it shouldn't get here.
        }
    });

    EXPECT_ANY_THROW({
        subpar::parallelize(
            255,
            2,
            []() -> bool {
                throw 1;
                return true; 
            },
            [&](size_t, int, int, bool&) -> void {
            }
        );
    });
}
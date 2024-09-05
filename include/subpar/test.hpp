#ifndef SUBPAR_TEST_HPP
#define SUBPAR_TEST_HPP

/**
 * @file test.hpp
 * @brief Utilities for testing parallelization.
 */

#include <random>
#include <algorithm>

namespace subpar {

/**
 * @brief An alternative to `parallelize_range()` for testing.
 *
 * This implements a very silly parallelization scheme that splits `num_tasks` into Q intervals where Q is `ceil(interval_scaling * num_workers)`.
 * Each interval is executed by `run_task_range()` in random order with a random worker in `[0, num_workers)`.
 * Each worker may execute zero, one or multiple (potentially non-contiguous and unordered) intervals.
 *
 * This function is intended for developers to set as `SUBPAR_CUSTOM_PARALLELIZE_RANGE` during unit testing of their own libraries.
 * The idea is to test that library code behaves correctly for a user-defined parallelization scheme with an arbitrarily complicated mapping of intervals to workers.
 * For example, code might incorrectly assume that the worker identity (i.e., `w`) is only associated with a single interval, which would be broken by this function.
 *
 * @tparam Task_ See the parameter of the same name in `parallelize_range()`.
 * @tparam Run_ See the parameter of the same name in `parallelize_range()`.
 *
 * @param num_workers See the argument of the same name in `parallelize_range()`.
 * @param num_tasks See the argument of the same name in `parallelize_range()`.
 * @param run_task_range See the argument of the same name in `parallelize_range()`.
 * @tparam interval_scaling Ratio of the number of intervals to the number of workers.
 */
template<typename Task_, typename Run_>
void test_parallelize_range(int num_workers, Task_ num_tasks, Run_ run_task_range, double interval_scaling = 1.5) {
    if (num_workers <= 0) {
        num_workers = 1;
    }

    int num_intervals = (num_workers * interval_scaling + 0.5);
    if (num_intervals <= 0) {
        num_intervals = 1;
    }

    Task_ tasks_per_worker = (num_tasks / num_intervals) + (num_tasks % num_intervals > 0); // just doing something quick and dirty, who cares.

    std::vector<int> interval_order(num_intervals);
    std::iota(interval_order.begin(), interval_order.end(), 0);
    std::mt19937_64 rng(num_intervals + num_tasks); // choosing a somewhat random-ish seed.
    std::shuffle(interval_order.begin(), interval_order.end(), rng);

    for (auto i : interval_order) {
        Task_ start = i * tasks_per_worker;
        if (start < num_tasks) {
            Task_ length = std::min(static_cast<Task_>(num_tasks - start), tasks_per_worker);
            run_task_range(rng() % num_workers, start, length);
        }
    }
}

}

#endif

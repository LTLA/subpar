#ifndef SUBPAR_RANGE_HPP
#define SUBPAR_RANGE_HPP

#include <limits>
#include <type_traits>

#ifndef SUBPAR_CUSTOM_PARALLELIZE_RANGE
#include <vector>
#include <stdexcept>
#include <thread>
#endif

#include "sanisizer/sanisizer.hpp"

/**
 * @file range.hpp
 * @brief Parallelize across a range of tasks.
 */

namespace subpar {

/**
 * @brief Adjust the number of workers to the number of tasks in `parallelize_range()`.
 *
 * It is not strictly necessary to run `sanitize_num_workers()` prior to `parallelize_range()` as the latter will automatically behave correctly with all inputs.
 * However, on occasion, applications need a better upper bound on the number of workers, e.g., to pre-allocate expensive per-worker data structures.
 * This upper bound can be obtained by `sanitize_num_workers()` to refine the number of workers prior to calling `parallelize_range()`.
 *
 * @tparam Task_ Integer type for the number of tasks.
 *
 * @param num_workers Number of workers.
 * This may be negative or zero.
 * @param num_tasks Number of tasks.
 * This should be a non-negative integer.
 *
 * @return A more suitable number of workers, possibly zero.
 * The return value of `sanitize_num_workers()` will be an upper bound to the return value of `parallelize_range()` with the same `num_workers` and `num_tasks`.
 */
template<typename Task_>
int sanitize_num_workers(const int num_workers, const Task_ num_tasks) {
    // This code mirrors the return logic in the default parallelize_range(), but would be an upper bound even with a custom SUBPAR_CUSTOM_PARALLELIZE_RANGE.
    // Remember that run_task_range must be called with a non-empty range so if num_tasks = 0, there is no choice but to not perform any calls.
    // Similarly, we can't perform more than one call if num_tasks = 1, and we can't perform more calls than there are workers.

    if (num_tasks <= 0) {
        return 0;
    }

    if (num_workers <= 1 || num_tasks == 1) {
        return 1;
    }

    return sanisizer::min(num_workers, num_tasks);
}

/**
 * @brief Parallelize a range of tasks across multiple workers.
 *
 * This function splits the integer sequence `[0, num_tasks)` into non-overlapping non-empty contiguous ranges.
 * Each range is passed to the user-supplied `run_task_range()` function for parallel execution by different workers via OpenMP (if available) or `<thread>` (otherwise).
 * Not all workers may be used, e.g., if `num_tasks < num_workers`, but each worker will process no more than one range.
 * By default, the ranges are evenly sized for efficient load-sharing across workers.
 * The partitioning of the ranges is also deterministic -
 * given the same `num_workers` and `num_tasks`,`parallelize_range()` will always call `run_task_range()` with the same combinations of arguments (`w`, `start` and `length`).
 * This avoids stochasticity in downstream applications that perform, e.g., reductions of floating-point results generated in each worker.
 *
 * The `SUBPAR_USES_OPENMP_RANGE` macro will be defined as 1 if and only if OpenMP was used in the default scheme.
 * Users can define the `SUBPAR_NO_OPENMP_RANGE` macro to force `parallelize_range()` to use `<thread>` even if OpenMP is available.
 * This is occasionally useful when OpenMP cannot be used in some parts of the application, e.g., with POSIX forks.
 *
 * Advanced users can substitute in their own parallelization scheme by defining `SUBPAR_CUSTOM_PARALLELIZE_RANGE` before including the **subpar** header.
 * For example, we might restrict the number of used workers to the number of physical cores available on the system,
 * or we might create task ranges of different lengths for targeted execution on performance or efficiency cores. 
 * `SUBPAR_CUSTOM_PARALLELIZE_RANGE` should be a function-like macro or the name of a function that accepts the same arguments as `parallelize_range()`,
 * partitions the tasks into `num_workers` or fewer ranges, calls `run_task_range()` on each task range, and returns the number of used workers.
 * All expectations for the arguments and return value for `parallelize_range()` are still applicable here.
 * Partitioning of task ranges should be deterministic but can vary across compute environments, e.g., with different numbers of available cores. 
 * Once the macro is defined, the custom scheme will be used instead of the default scheme whenever `parallelize_range()` is called.
 *
 * If `nothrow_ = true`, exception handling is omitted from the default parallelization scheme.
 * This avoids some unnecessary work when the caller knows that `run_task_range()` will never throw. 
 * For custom schemes, if `SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW` is defined, it will be used if `nothrow_ = true`;
 * otherwise, `SUBPAR_CUSTOM_PARALLELIZE_RANGE` will continue to be used.
 * Any definition of `SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW` should follow the same rules described above for `SUBPAR_CUSTOM_PARALLELIZE_RANGE`.
 *
 * @tparam nothrow_ Whether the `Run_` function cannot throw an exception.
 * @tparam Task_ Integer type for the number of tasks.
 * @tparam Run_ Function that accepts three arguments:
 * - `w`, the identity of the worker executing this task range.
 *   This will be passed as an `int` in `[0, num_workers)`.
 * - `start`, the start index of the task range.
 *   This will be passed as a `Task_` in `[0, num_tasks)`.
 * - `length`, the number of tasks in the task range.
 *   This will be passed as a `Task_` in `(0, num_tasks)`, i.e., it is guaranteed to be positive.
 * .
 * Any return value is ignored.
 *
 * @param num_workers Number of workers.
 * This should be a positive integer.
 * Any zero or negative values are treated as 1.
 * (See also `sanitize_num_workers()`.)
 * @param num_tasks Number of tasks.
 * This should be a non-negative integer.
 * @param run_task_range Function to iterate over a range of tasks within a worker.
 * This will be called no more than once in each worker.
 * In each call:
 * - `w` is guaranteed to be in `[0, K)` where `K` is the return value of `parallelize_range()`.
 *   `K` itself is guaranteed to be no greater than `num_workers`.
 * - `[start, start + length)` is guaranteed to be a non-empty range of tasks that lies in `[0, num_tasks)`.
 *   It will not overlap with the task range used in any other call to `run_task_range()` in the same call to `parallelize_range()`.
 * .
 * This function may throw an exception if `nothrow_ = false`.
 *
 * @return The number of workers (`K`) that were actually used. 
 * This is guaranteed to be no greater than `num_workers` (or 1, if `num_workers` is not positive).
 * It can be assumed that `run_task_range` was called once for each of `[0, 1, ..., K-1]`, where the union of task ranges across all `K` workers is `[0, num_tasks)`.
 */
template<bool nothrow_ = false, typename Task_, class Run_>
int parallelize_range(int num_workers, const Task_ num_tasks, const Run_ run_task_range) {
#ifdef SUBPAR_CUSTOM_PARALLELIZE_RANGE
    if constexpr(nothrow_) {
#ifdef SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW
        return SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW(num_workers, num_tasks, run_task_range);
#else
        return SUBPAR_CUSTOM_PARALLELIZE_RANGE(num_workers, num_tasks, run_task_range);
#endif
    } else {
        return SUBPAR_CUSTOM_PARALLELIZE_RANGE(num_workers, num_tasks, run_task_range);
    }

#else
    if (num_tasks <= 0) {
        return 0;
    }

    if (num_workers <= 1 || num_tasks == 1) {
        run_task_range(0, 0, num_tasks);
        return 1;
    }

    // All workers with indices below 'remainder' get an extra task to fill up the remainder.
    Task_ tasks_per_worker = 1;
    int remainder = 0;
    if (sanisizer::is_greater_than_or_equal(num_workers, num_tasks)) {
        num_workers = num_tasks;
    } else {
        tasks_per_worker = num_tasks / num_workers;
        remainder = num_tasks % num_workers;
    }

    const auto get_start = [&tasks_per_worker,&remainder](const int w) -> Task_ {
        // Need to shift the start by the number of previous 'w' that added a remainder.
        return w * tasks_per_worker + (w < remainder ? w : remainder);
    };

    const auto get_length = [&tasks_per_worker,&remainder](const int w) -> Task_ {
        return tasks_per_worker + (w < remainder); 
    };

    // Avoid instantiating a vector if it is known that the function can't throw.
    auto errors = [&]{
        if constexpr(nothrow_) {
            return true;
        } else {
            return sanisizer::create<std::vector<std::exception_ptr> >(num_workers);
        }
    }();

#if defined(_OPENMP) && !defined(SUBPAR_NO_OPENMP_RANGE) && !defined(SUBPAR_NO_OPENMP)
#define SUBPAR_USES_OPENMP 1
#define SUBPAR_USES_OPENMP_RANGE 1

    // OpenMP doesn't guarantee that we'll actually start the specified number of workers,
    // so we need to do a loop here to ensure that each task range is executed.
    #pragma omp parallel for num_threads(num_workers)
    for (int w = 0; w < num_workers; ++w) {
        const Task_ start = get_start(w);
        const Task_ length = get_length(w);

        if constexpr(nothrow_) {
            run_task_range(w, start, length);
        } else {
            try { 
                run_task_range(w, start, length);
            } catch (...) {
                errors[w] = std::current_exception();
            }
        }
    }

#else
// Wiping it out, just in case.
#undef SUBPAR_USES_OPENMP
#undef SUBPAR_USES_OPENMP_RANGE

    // We run the first job on the current thread, to avoid having to spin up an unnecessary worker.
    std::vector<std::thread> workers;
    sanisizer::reserve(workers, num_workers - 1); // preallocate to ensure we don't get alloc errors during emplace_back().

    for (int w = 1; w < num_workers; ++w) {
        const Task_ start = get_start(w);
        const Task_ length = get_length(w);

        if constexpr(nothrow_) {
            workers.emplace_back(run_task_range, w, start, length);
        } else {
            workers.emplace_back([&run_task_range,&errors](int w, Task_ start, Task_ length) -> void {
                try {
                    run_task_range(w, start, length);
                } catch (...) {
                    errors[w] = std::current_exception();
                }
            }, w, start, length);
        }
    }

    {
        const Task_ start = get_start(0);
        const Task_ length = get_length(0);

        if constexpr(nothrow_) {
            run_task_range(0, start, length);
        } else {
            try {
                run_task_range(0, start, length);
            } catch (...) {
                errors[0] = std::current_exception();
            }
        }
    }

    for (auto& wrk : workers) {
        wrk.join();
    }
#endif

    if constexpr(!nothrow_) {
        for (const auto& e : errors) {
            if (e) {
                std::rethrow_exception(e);
            }
        }
    }
#endif

    return num_workers;
}

/**
 * @cond
 */
// Back-compatibility only.
template<typename Task_, class Run_>
void parallelize(int num_workers, Task_ num_tasks, Run_ run_task_range) {
    parallelize_range<false, Task_, Run_>(num_workers, num_tasks, std::move(run_task_range));
}
/**
 * @endcond
 */

}

#endif

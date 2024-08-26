#ifndef SUBPAR_SUBPAR_HPP
#define SUBPAR_SUBPAR_HPP

#ifndef SUBPAR_CUSTOM_PARALLEL
#include <vector>
#include <stdexcept>
#include <thread>
#endif

/**
 * @file subpar.hpp
 * @brief Substitutable parallelization functions.
 */

/**
 * @namespace subpar
 * @brief Substitutable parallelization functions.
 */
namespace subpar {

/**
 * Parallelize a range of tasks across multiple workers (i.e., workers).
 * The aim is to split tasks in `[0, num_tasks)` into intervals that are executed by different workers.
 * By default, we create `num_threads` evenly-sized intervals that are distributed via OpenMP (if available) or `<thread>` (otherwise).
 * This is done using OpenMP if available, otherwise `<worker>` is used.
 *
 * Advanced users can substitute in their own parallelization scheme by defining a `SUBPAR_CUSTOM_PARALLEL` function-like macro.
 * This should accept the same arguments as `parallelize()` and will be used instead of OpenMP/`<thread>` whenever `parallelize()` is called.
 * Macro authors should note the expectations on `per_worker_setup()` and `run_task_range()`.
 *
 * @tparam Task_ Integer type for the number of tasks.
 * @tparam Setup_ Function that accepts no arguments and returns an arbitrary "workspace" object.
 * @tparam Run_ Function that accepts four arguments:
 * - `w`, the worker number executing this task range.
 *   This will be passed as an `int`.
 * - `start`, the start index of the task range.
 *   This will be passed as a `Task_`.
 * - `length`, the number of tasks in the task range.
 *   This will be passed as a `Task_`.
 * - `workspace`, an instance of the workspace object returned by the `Setup_` function.
 * .
 * Any return value is ignored.
 *
 * @param num_workers Number of workers.
 * This should be a positive integer.
 * Any zero or negative values are treated as 1.
 * @param num_tasks Number of tasks.
 * This should be a non-negative integer.
 * @param per_worker_setup Function that creates a per-worker workspace object.
 * This function may be zero, one or multiple times per worker.
 * The same workspace object may be re-used across multiple `run_task_range()` calls within a worker.
 * This function may throw an exception.
 * @param run_task_range Function to iterate over a range of tasks within a single worker.
 * This may be called zero, one or multiple times for a single worker.
 * In each call:
 * - `w` is guaranteed to be in `[0, num_workers)`.
 * - `[start, start + length)` is guaranteed to be a valid and non-empty range of tasks that does not overlap with any other range in any other call to `run_task_range()`.
 * - `workspace` may have been used by a previous call to `run_task_range()` in the same worker.
 * .
 * This function may throw an exception.
 */
template<typename Task_, class Setup_, class Run_>
void parallelize(int num_workers, Task_ num_tasks, Setup_ per_worker_setup, Run_ run_task_range) {
#ifdef SUBPAR_CUSTOM_PARALLEL
    SUBPAR_CUSTOM_PARALLEL(num_workers, num_tasks, std::move(per_worker_setup), std::move(run_task_range));

#else
    if (num_tasks == 0) {
        return;
    }

    if (num_workers <= 1 || num_tasks == 1) {
        auto workspace = per_worker_setup();
        run_task_range(0, 0, num_tasks, workspace);
        return;
    }

    // All workers with indices below 'remainder' get an extra task to fill up the remainder.
    Task_ tasks_per_worker = num_tasks / num_workers;
    int remainder = num_tasks % num_workers;
    if (tasks_per_worker == 0) {
        num_workers = num_tasks;
        tasks_per_worker = 1;
        remainder = 0;
    }

    std::vector<std::exception_ptr> errors(num_workers);

#ifdef _OPENMP
    // OpenMP doesn't guarantee that we'll actually start 'num_workers' workers,
    // so we need to do a loop here to ensure that each task range is executed.
    #pragma omp parallel for num_threads(num_workers)
    for (int w = 0; w < num_workers; ++w) {
        try { 
            auto workspace = per_worker_setup();
            Task_ start = w * tasks_per_worker + (w < remainder ? w : remainder); // need to shift the start by the number of previous 't' that added a remainder.
            Task_ length = tasks_per_worker + (w < remainder);
            run_task_range(w, start, length, workspace);
        } catch (...) {
            errors[w] = std::current_exception();
        }
    }

#else
    Task_ start = 0;
    std::vector<std::thread> workers;
    workers.reserve(num_workers);

    for (int w = 0; w < num_workers; ++w) {
        Task_ length = tasks_per_worker + (w < remainder); 
        workers.emplace_back([&per_worker_setup,&run_task_range,&errors](int w, Task_ start, Task_ length) -> void {
            try {
                auto workspace = per_worker_setup();
                run_task_range(w, start, length, workspace);
            } catch (...) {
                errors[w] = std::current_exception();
            }
        }, w, start, length);
        start += length;
    }

    for (auto& wrk : workers) {
        wrk.join();
    }
#endif

    for (const auto& e : errors) {
        if (e) {
            std::rethrow_exception(e);
        }
    }
#endif
}

}

#endif

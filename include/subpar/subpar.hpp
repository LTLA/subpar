#ifndef SUBPAR_SUBPAR_HPP
#define SUBPAR_SUBPAR_HPP

#ifndef SUBPAR_CUSTOM_PARALLEL
#include <vector>
#include <stdexcept>
#include <thread>
#include <string>
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
 * Advanced users can substitute their own parallelization scheme by defining a `SUBPAR_CUSTOM_PARALLEL` function-like macro.
 * This should accept the same arguments as `parallelize()` and will be used instead of OpenMP/`<thread>` whenever `parallelize()` is called.
 * The `SUBPAR_CUSTOM_PARALLEL` macro can be used to inject other parallelization mechanisms like Intel's TBB, TinyThread, Boost, etc.
 *
 * 
 *
 * @tparam Task_ Integer type for the number of tasks.
 * @tparam Setup_ Function that accepts no arguments and returns an arbitrary "workspace" object.
 * @tparam Run_ Function that accepts four arguments:
 * - `t`, the worker number executing this task range.
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
 * This will be executed no more than once per worker.
 * The same workspace object may be re-used across multiple `run_task_range()` calls.
 * @param run_task_range Function to iterate over a range of tasks within a single worker.
 * This may be called zero, one or multiple times for a single worker on the same workspace object.
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

    std::vector<std::string> errors(num_workers);

#ifdef _OPENMP
    #pragma omp parallel num_threads(num_workers)
    {
        auto workspace = per_worker_setup();

        // OpenMP doesn't guarantee that we'll actually start 'num_workers' workers,
        // so we need to do a loop here to ensure that each task range is executed.
        #pragma omp for
        for (int t = 0; t < num_workers; ++t) {
            try { 
                Task_ start = t * tasks_per_worker + (t < remainder ? t : remainder); // need to shift the start by the number of previous 't' that added a remainder.
                Task_ length = tasks_per_worker + (t < remainder);
                run_task_range(t, start, length, workspace);
            } catch (std::exception& e) {
                errors[t] = e.what();
            } catch (...) {
                errors[t] = "unknown error in worker " + std::to_string(t);
            }
        }
    }

#else
    Task_ start = 0;
    std::vector<std::thread> workers;
    workers.reserve(num_workers);

    for (int t = 0; t < num_workers; ++t) {
        Task_ length = tasks_per_worker + (t < remainder); 
        workers.emplace_back([&per_worker_setup,&run_task_range,&errors](int t, Task_ start, Task_ length) -> void {
            try {
                auto workspace = per_worker_setup();
                run_task_range(t, start, length, workspace);
            } catch (std::exception& e) {
                errors[t] = e.what();
            } catch (...) {
                errors[t] = "unknown error in worker " + std::to_string(t);
            }
        }, t, start, length);
        start += length;
    }

    for (auto& wrk : workers) {
        wrk.join();
    }
#endif

    for (const auto& e : errors) {
        if (!e.empty()) {
            throw std::runtime_error(e);
        }
    }
#endif
}

}

#endif

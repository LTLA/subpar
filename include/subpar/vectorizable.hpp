#ifndef SUBPAR_VECTORIZABLE_HPP
#define SUBPAR_VECTORIZABLE_HPP

/**
 * @file vectorizable.hpp
 * @brief Indicate that a loop is vectorizable.
 */

#ifndef SUBPAR_VECTORIZABLE
#if defined(__clang__)
#define SUBPAR_VECTORIZABLE _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__) || defined(__GNUG__)
#define SUBPAR_VECTORIZABLE _Pragma("GCC ivdep")
#elif defined(_MSC_VER)
#define SUBPAR_VECTORIZABLE _Pragma("loop(ivdep)")
#else
/**
 * Indicate a `for` loop can be safely vectorized by promising that there are no data dependencies between loop iterations.
 * This should be added above the loop and is replaced by a compiler-specific pragma.
 * Unlike other directives like OpenMP SIMD, use of this macro does not force vectorization of the loop.
 * Rather, the decision to auto-vectorize or not is left to the compiler and its cost model.
 *
 * The nature of this macro means that the exact interpretation of "safe to vectorize" is compiler-specific.
 * For example, are all data dependencies ignored? Or just non-proven ones?
 * To ensure portable use of this macro, we recommend the most conservative interpretation:
 *
 * - The `for` loop should operate by modifying a single integer value by a constant.
 *   Avoid iterators as these may hold extra mutable state that may interfere with correct vectorization.
 * - Different iterations of the loop should be executable in any order and/or concurrently without affecting the results.
 *   This implies that iterations should not modify a shared value, e.g., no reductions.
 * - Read and write operations should only be performed to arrays, via pointers or standard wrappers around them - typically `std::vector`.
 *   This avoids unanticipated side-effects from other containers' access operators, as implied by the previous constraint. 
 */
#define SUBPAR_VECTORIZABLE
#endif
#endif

#endif

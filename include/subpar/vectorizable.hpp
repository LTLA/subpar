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
 * Indicate a loop can be safely vectorized by promising that there are no dependencies between loop iterations.
 * This should be added above the loop and is replaced by a compiler-specific pragma.
 * Use of this macro does not force vectorization of the loop;
 * the decision to vectorize or not is left to the compiler's cost model.
 */
#define SUBPAR_VECTORIZABLE
#endif
#endif

#endif

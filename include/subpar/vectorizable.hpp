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
 * Note that the use of this macro does not force vectorization.
 * Rather, it only provides a hint to the compiler that auto-vectorization is possible;
 * the decision to vectorize or not is left to the compiler's cost model.
 *
 * This pragma is most useful for indicating that indexed access operations are independent,
 * as the compiler has no other way of knowing that an access in an iteration does not alter the value for the next iteration.
 * However, it can also be beneficial for loops where auto-vectorization is relatively trivial,
 * as it eliminates the need for pre-loop protection against aliasing pointers.
 */
#define SUBPAR_VECTORIZABLE
#endif
#endif

#endif

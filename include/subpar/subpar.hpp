#ifndef SUBPAR_SUBPAR_HPP
#define SUBPAR_SUBPAR_HPP

#if defined(_OPENMP) && !defined(SUBPAR_NO_OPENMP)
#define SUBPAR_USES_OPENMP 1
#else
// Wiping it out, just in case.
#undef SUBPAR_USES_OPENMP
#endif

#include "range.hpp"

/**
 * @file subpar.hpp
 * @brief Substitutable parallelization functions.
 */

/**
 * @namespace subpar
 * @brief Substitutable parallelization functions.
 */
namespace subpar {}

#endif

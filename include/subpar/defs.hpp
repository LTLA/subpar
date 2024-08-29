#ifndef SUBPAR_DEFS_HPP
#define SUBPAR_DEFS_HPP

#if defined(_OPENMP) && !defined(SUBPAR_NO_OPENMP)
#define SUBPAR_USES_OPENMP 1
#else
// Wiping it out, just in case.
#undef SUBPAR_USES_OPENMP
#endif

#endif

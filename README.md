# Substitutable parallelization for C++ libraries

![Unit tests](https://github.com/LTLA/subpar/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/LTLA/subpar/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/LTLA/subpar/branch/master/graph/badge.svg?token=GByG4StuqU)](https://codecov.io/gh/LTLA/subpar)

## Overview

This repository implements a simple `parallelize_range()` function that can be used throughout all of my C++ libaries.
By default, it just splits the range of tasks into equi-sized intervals that are executed by different workers.
OpenMP is used if it is available, otherwise we spin up a new `std::thread` for each interval.
Advanced users can define a macro to instruct `parallelize_range()` to use their own parallelization mechanism.
This allows applications to quickly substitute all instances of `parallelize_range()` across all libraries with something more appropriate.

## Quick start

The `subpar::parallelize_range()` function requires the number of workers, the number of tasks,
and a function that iterates over the range of tasks and executes them in a single worker.

```cpp
#include "subpar/subpar.hpp"

subpar::parallelize_range(
    /* num_workers = */ 10,
    /* num_tasks = */ 12345,
    /* run = */ [&](int worker, int start, int len) {
        // ... do some per-worker set-up ...
        for (int task = start, end = start + len; task < end; ++task) {
            // ...process each task ...
        }
    }
);
```

`subpar::parallelize_range()` functions will correctly handle exceptions thrown in the user-supplied function.
This comes with some slight overhead so the user can set the `nothrow_` template parameter to `false` if they can guarantee that their supplied functions will not throw.

On occasion, the user has already assigned a task to each worker outside of **subpar**.
This is handled by `subpar::parallelize_simple()`, which will execute each task in its own worker without the overhead of task range partitioning.

```cpp
#include "subpar/subpar.hpp"

subpar::parallelize_simple(
    /* num_tasks = */ 4,
    /* run = */ [&](int task) {
        // ...process each task ...
    }
);
```

Check out the [reference documentation](https://ltla.github.io/subpar) for more details.

## Custom parallelization 

If the `SUBPAR_CUSTOM_PARALLELIZE_RANGE` function-like macro is defined, it is used in place of the default behavior whenever `subpar::parallelize_range()` is called.
The macro should accept exactly the same arguments as `subpar::parallelize_range()`, but the macro author is now responsible for distributing tasks among workers.
This can be used to inject other parallelization mechanisms like Intel's TBB, TinyThread, Boost, etc.
The same approach can be applied to `subpar::parallelize_simple()` via the `SUBPAR_CUSTOM_PARALLELIZE_SIMPLE` macro.

Users can additionally define the `SUBPAR_CUSTOM_PARALLELIZE_RANGE_NOTHROW` or `SUBPAR_CUSTOM_PARALLELIZE_SIMPLE_NOTHROW` macros.
These will be used in place of their non-`NOTHROW` counterparts in the `nothrow_ = true` case,
providing an optimization opportunity for custom parallelization schemes when exception handling is not required.
Note that the non-`NOTHROW` macros still need to be defined before the `NOTHROW` macros can be used, even if all calls to **subpar** functions use `nothrow_ = true`.

## Building projects 

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  subpar
  GIT_REPOSITORY https://github.com/LTLA/subpar
  GIT_TAG master # or any version of interest 
)

FetchContent_MakeAvailable(subpar)
```

Then you can link to **subpar** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe subpar)

# For libaries
target_link_libraries(mylib INTERFACE subpar)
```

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DSUBPAR_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(ltla_subpar CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE ltla::subpar)
```

### Manual

If you're not using CMake, the simple approach is to just copy the files in the `include/` subdirectory - 
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.

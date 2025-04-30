include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/ff233bdd4cac0a0bf6e5cd45bda3406814cb2796.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

set(CODE_COVERAGE OFF CACHE BOOL "Enable coverage testing")
set(DO_CODE_COVERAGE OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(DO_CODE_COVERAGE ON)
endif()

macro(decorate_executable target)
    target_link_libraries(
        ${target}
        gtest_main
        subpar
    )

    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)

    if(DO_CODE_COVERAGE)
        target_compile_options(${target} PRIVATE -O0 -g --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()

    gtest_discover_tests(${target})
endmacro()

macro(create_executable target)
    add_executable(
        ${target}
        src/range.cpp
        src/simple.cpp
    )
    decorate_executable(${target})
endmacro()

create_executable(libtest)

create_executable(custest)
target_compile_definitions(custest PRIVATE CUSTOM_PARALLEL_TEST=1)

create_executable(cusnothrowtest)
target_compile_definitions(cusnothrowtest PRIVATE CUSTOM_PARALLEL_TEST=1 CUSTOM_PARALLEL_TEST_NOTHROW=1)

find_package(OpenMP)
if(OpenMP_FOUND)
    create_executable(omptest)
    target_link_libraries(omptest OpenMP::OpenMP_CXX)
endif()

cmake_minimum_required(VERSION 3.24)

project(subpar
    VERSION 0.2.1
    DESCRIPTION "Substitutable parallelization for C++ libraries"
    LANGUAGES CXX)

add_library(subpar INTERFACE)
add_library(ltla::subpar ALIAS subpar)

target_compile_features(subpar INTERFACE cxx_std_17)

include(GNUInstallDirs)
target_include_directories(subpar INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ltla_subpar>"
)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(SUBPAR_TESTS "Build subpar's test suite." ON)
else()
    option(SUBPAR_TESTS "Build subpar's test suite." OFF)
endif()

if(SUBPAR_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Installing for find_package.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ltla_subpar)

install(TARGETS subpar 
    EXPORT subparTargets)

install(EXPORT subparTargets
    FILE ltla_subparTargets.cmake
    NAMESPACE ltla::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_subpar)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_subparConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_subpar)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ltla_subparConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ltla_subparConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_subparConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_subpar)


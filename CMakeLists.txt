# ============================================================================
# Project Configuration
# ============================================================================
cmake_minimum_required(VERSION 3.15)
project(CryptographicSecretSharing 
        VERSION 1.0.0
        DESCRIPTION "Shamir's Secret Sharing implementation in C"
        LANGUAGES C)

# ============================================================================
# C Standard and Compiler Settings
# ============================================================================
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ============================================================================
# Build Type and Compiler Flags
# ============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings and optimizations
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

# Debug flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
# Release flags
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# ============================================================================
# Dependencies - Find libsodium
# ============================================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

if(SODIUM_FOUND)
    message(STATUS "Found libsodium: ${SODIUM_LIBRARIES}")
    include_directories(${SODIUM_INCLUDE_DIRS})
    link_directories(${SODIUM_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "libsodium not found!  Install with: brew install libsodium")
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# ============================================================================
# Source Files (Only include files that exist)
# ============================================================================
set(CORE_SOURCES
    src/core/field_arithmetic.c
    # src/core/secret_sharing. c        # TODO: Not created yet
    # src/core/polynomial.c             # TODO: Not created yet
)

# Temporarily comment out until we create these files
# set(ALGORITHM_SOURCES
#     src/algorithms/shamir.c
# )
# 
# set(UTIL_SOURCES
#     src/utils/random.c
#     src/utils/error.c
# )

set(ALL_SOURCES
    ${CORE_SOURCES}
    # ${ALGORITHM_SOURCES}              # TODO:  Uncomment later
    # ${UTIL_SOURCES}                   # TODO: Uncomment later
)

# ============================================================================
# Library Target - Build as static library
# ============================================================================
add_library(sss STATIC ${ALL_SOURCES})

target_include_directories(sss PUBLIC
    $<BUILD_INTERFACE: ${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(sss PUBLIC ${SODIUM_LIBRARIES})

# ============================================================================
# Testing (Commented out until we create test files)
# ============================================================================
# enable_testing()
# 
# add_executable(test_unit
#     tests/unit/test_secret_sharing.c
#     tests/unit/test_polynomial.c
# )
# 
# target_link_libraries(test_unit PRIVATE sss)
# add_test(NAME UnitTests COMMAND test_unit)

# ============================================================================
# Examples (Commented out until we create example files)
# ============================================================================
# add_executable(basic_example examples/basic/simple_split.c)
# target_link_libraries(basic_example PRIVATE sss)
# 
# add_executable(advanced_example examples/advanced/threshold_demo.c)
# target_link_libraries(advanced_example PRIVATE sss)

# ============================================================================
# Installation Rules
# ============================================================================
install(TARGETS sss
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include/sss
        DESTINATION include)
CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

# Project config
project(
    benchmark
    LANGUAGES CXX
)

# Source files
set(
    BENCHMARK_CASE_SOURCE_LIST
    BlocksGeneration.cpp
    ZhuVolumeLoss.cpp
)

# Benchmarks
set(TEST_TARGET_PREFIX "benchmark")
foreach(BENCKMARK_CASE_SOURCE IN ITEMS ${BENCHMARK_CASE_SOURCE_LIST})
    cmake_path(GET BENCKMARK_CASE_SOURCE STEM TEST_CASE_NAME)
    set(TEST_TARGET "${TEST_TARGET_PREFIX}_${TEST_CASE_NAME}")
    add_executable(${TEST_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/${BENCKMARK_CASE_SOURCE})
    target_link_libraries(
        ${TEST_TARGET}
        PRIVATE
            sclp
            Catch2
    )
endforeach()



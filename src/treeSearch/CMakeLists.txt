cmake_minimum_required(VERSION 3.26)
project(bb3d_sclp_reference LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
get_filename_component(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(UPSTREAM "${ROOT}/third_party/sclp")
set(BOOST_ROOT "${ROOT}/build/sclp-original/boost_1_83_0" CACHE PATH "Boost 1.83 source tree")
if(NOT EXISTS "${BOOST_ROOT}/boost/version.hpp")
  message(FATAL_ERROR "Download/extract Boost 1.83 as documented in README.md")
endif()
add_subdirectory("${UPSTREAM}/dependency/fmt" "fmt" EXCLUDE_FROM_ALL)
add_library(reference_core STATIC
  "${UPSTREAM}/src/EmptySpaceCollection/EmptySpaceCutter.cpp"
  "${UPSTREAM}/src/Geometry/AllocatedRectangularCuboid.cpp"
  "${UPSTREAM}/src/Geometry/Area.cpp"
  "${UPSTREAM}/src/Geometry/Orientation.cpp"
  "${UPSTREAM}/src/Types/Timer.cpp"
  "${BOOST_ROOT}/libs/timer/src/cpu_timer.cpp"
  "${BOOST_ROOT}/libs/timer/src/auto_timers_construction.cpp")
if(WIN32)
  # glibc x86_64 int_fast16_t is 64-bit; MinGW's is 16-bit, causing
  # integer promotion/template deduction failures in the original code.
  file(READ "${UPSTREAM}/include/sclp/Types/Measure.hpp" MEASURE)
  string(REPLACE "using CoordinateType = std::int_fast16_t;" "using CoordinateType = std::int64_t;" MEASURE "${MEASURE}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compat/sclp/Types")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/compat/sclp/Types/Measure.hpp" "${MEASURE}")
  target_include_directories(reference_core BEFORE PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/compat")
endif()
target_include_directories(reference_core PUBLIC
  "${UPSTREAM}/include" "${BOOST_ROOT}"
  "${UPSTREAM}/dependency/json/include"
  "${UPSTREAM}/dependency/cxxopts/include")
target_compile_definitions(reference_core PUBLIC BOOST_TIMER_STATIC_LINK BOOST_ALL_NO_LIB)
target_link_libraries(reference_core PUBLIC fmt::fmt)
add_executable(sclp_reference reference_main.cxx)
target_link_libraries(sclp_reference PRIVATE reference_core)
# Original CLI is also built to check parity without the extra result metadata.
add_executable(sclp_original "${UPSTREAM}/app/allocate.cpp")
target_link_libraries(sclp_original PRIVATE reference_core)

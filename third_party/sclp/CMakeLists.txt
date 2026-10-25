CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

# Project config
project(
    sclp
    LANGUAGES CXX
)

# Build options
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Dependencies
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG COMPONENTS timer container REQUIRED)
add_subdirectory(dependency/fmt)
add_subdirectory(dependency/json)
add_subdirectory(dependency/cxxopts)
add_subdirectory(dependency/Catch2)

# Source files
set(SCLP_INCLUDE_DIR "include")
set(SCLP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(
    SCLP_SOURCE_FILES
    ${SCLP_SOURCE_DIR}/EmptySpaceCollection/EmptySpaceCutter.cpp
    ${SCLP_SOURCE_DIR}/Geometry/AllocatedRectangularCuboid.cpp
    ${SCLP_SOURCE_DIR}/Geometry/Area.cpp
    ${SCLP_SOURCE_DIR}/Geometry/Orientation.cpp
    ${SCLP_SOURCE_DIR}/Types/Timer.cpp
)

# Setup library
add_library(${PROJECT_NAME} STATIC ${SCLP_SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC -Werror -Wall -Wextra -pedantic-errors -Wno-error=ignored-qualifiers)
target_include_directories(${PROJECT_NAME} PUBLIC ${SCLP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Boost::boost Boost::timer
    fmt::fmt
    nlohmann_json
    cxxopts
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PUBLIC -ggdb3)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SCLP_DEBUG)
endif()

# other project files
include (CTest)
add_subdirectory(test)
add_subdirectory(benchmark)
add_subdirectory(app)

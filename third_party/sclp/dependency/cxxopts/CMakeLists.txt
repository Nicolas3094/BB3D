CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

project(
    cxxopts
    LANGUAGES CXX
    VERSION 3.1.1
)

add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

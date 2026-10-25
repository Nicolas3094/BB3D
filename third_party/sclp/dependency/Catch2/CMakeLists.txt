CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

project(
    Catch2
    LANGUAGES CXX
    VERSION 3.4.0
)

add_library(Catch2 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/catch.cpp)
target_include_directories(Catch2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

project(
    nlohmann_json
    LANGUAGES CXX
    VERSION 3.11.2
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

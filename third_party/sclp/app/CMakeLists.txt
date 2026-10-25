CMAKE_MINIMUM_REQUIRED(VERSION 3.26)

project(
    app
    LANGUAGES CXX
)

# Allocate
set(
    ALLOCATE_APP_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/allocate.cpp"
)
add_executable(allocate ${ALLOCATE_APP_SRC_FILES})
target_link_libraries(allocate PUBLIC sclp)

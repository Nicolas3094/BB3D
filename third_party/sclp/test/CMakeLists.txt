cmake_minimum_required(VERSION 3.24)

# Project config
project(
  test
  LANGUAGES CXX
)

# Tests
set(
    TEST_CASE_SOURCE_LIST
    Dependency/ArrayCompare.test.cpp
    Dependency/Catch.test.cpp
    Dependency/Json.test.cpp
    Dependency/fmt.test.cpp
    Allocation/ArayaVolumeLoss.test.cpp
    Allocation/BsgSearch.test.cpp
    Allocation/DoubleEffort.test.cpp
    Allocation/GuillotineBlockGenerator.test.cpp
    Allocation/GuillotineBlocksTimeout.test.cpp
    Allocation/State.test.cpp
    Allocation/ZhuVolumeLoss.test.cpp
    Constraint/LargeObjectSpaceConstraint.test.cpp
    Constraint/NonEmptyPackingConstraint.test.cpp
    Constraint/OrientationConstraint.test.cpp
    Constraint/OverlapConstraint.test.cpp
    Constraint/QuantityConstraint.test.cpp
    Constraint/AllConstraints.test.cpp
    EmptySpaceCollection/EmptySpaceCollection.test.cpp
    EmptySpaceCollection/EmptySpaceCutter.test.cpp
    Geometry/AllocatedObject.test.cpp
    Geometry/AllocatedRectangularCuboid.test.cpp
    Geometry/Anchor.test.cpp
    Geometry/Anchor_corner_selection.test.cpp
    Geometry/Area.test.cpp
    Geometry/ContinuousVector.test.cpp
    Geometry/DiscreteVector.test.cpp
    Geometry/DiscreteVector2D.test.cpp
    Geometry/Orientation.test.cpp
    Geometry/PlaneVector.test.cpp
    Geometry/PseudoContact.test.cpp
    Geometry/RectangularCuboid.test.cpp
    IO/FileLoader.test.cpp
    IO/InputReader.test.cpp
    Packing/AllocatedSmallItem.test.cpp
    Packing/Block.test.cpp
    Packing/Input.test.cpp
    Packing/Packing.test.cpp
    Packing/SmallItem.test.cpp
    Types/Hash.test.cpp
)

foreach(TEST_CASE_SOURCE IN ITEMS ${TEST_CASE_SOURCE_LIST})
    cmake_path(GET TEST_CASE_SOURCE STEM LAST_ONLY TEST_CASE_NAME)
    set(TEST_TARGET "${TEST_CASE_NAME}")
    add_executable(${TEST_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_CASE_SOURCE})
    target_link_libraries(
        ${TEST_TARGET}
        PRIVATE sclp Catch2
    )
    add_test(
        NAME ${TEST_TARGET}
        COMMAND ${TEST_TARGET}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
endforeach()

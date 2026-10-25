#include <catch.hpp>

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/Anchor.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

/**
 * Anchor distances for the two allocated rectangular cuboids:
 *
 * measurement: {2, 47, 29}, initial_position: {23, 13, 83}
 * measurement: {61, 97, 37}, initial_position: {7, 11, 43}
 *
 * | corner             | diffs    | anchor distance |
 * | :----------------: | :------: | :-------------: |
 * | BACK_RIGHT_BOTTOM  | 16  2 40 |  2 16 40        |
 * | FRONT_RIGHT_BOTTOM | 43  2 40 |  2 40 43        |
 * | BACK_LEFT_BOTTOM   | 16 48 40 | 16 40 48        |
 * | FRONT_LEFT_BOTTOM  | 43 48 40 | 40 43 48        |
 * | BACK_RIGHT_TOP     | 16  2 32 |  2 16 32        |
 * | FRONT_RIGHT_TOP    | 43  2 32 |  2 32 43        |
 * | BACK_LEFT_TOP      | 16 48 32 | 16 32 48        |
 * | FRONT_LEFT_TOP     | 43 48 32 | 32 43 48        |
 *
 */

TEST_CASE("anchor_distances", "[Anchor][Geometry]") {
    auto const lhs = AllocatedRectangularCuboid({2, 47, 29}, {23, 13, 83});
    auto const rhs = AllocatedRectangularCuboid({61, 97, 37}, {7, 11, 43});
    auto const actual = Anchor::value_of(lhs, rhs);
    // auto const distances = std::vector<Anchor::Distance>{
    //     // those are the anchor distances when not sorted
    //     2 + 16 + 40, // BACK_RIGHT_BOTTOM
    //     2 + 16 + 32, // BACK_RIGHT_TOP
    //     16 + 40 + 48, // BACK_LEFT_BOTTOM
    //     16 + 32 + 48, // BACK_LEFT_TOP
    //     2 + 40 + 43, // FRONT_RIGHT_BOTTOM
    //     2 + 32 + 43, // FRONT_RIGHT_TOP
    //     40 + 43 + 48, // FRONT_LEFT_BOTTOM
    //     32 + 43 + 48, // FRONT_LEFT_TOP
    // };
    auto const expected = decltype(actual){
        2 + 16 + 32,
        Corner::BACK_RIGHT_TOP};
    CHECK(actual == expected);
}

TEST_CASE("value_of", "[Anchor][Geometry]") {
    auto const lhs = AllocatedRectangularCuboid({2, 47, 29}, {23, 13, 83});
    auto const rhs = AllocatedRectangularCuboid({61, 97, 37}, {7, 11, 43});
    auto const actual = Anchor::value_of(lhs, rhs);
    auto const expected = decltype(actual){
        2 + 16 + 32,
        Corner::BACK_RIGHT_TOP};
    CHECK(actual == expected);
}

TEST_CASE("tie cases", "[Anchor][Geometry]") {
    auto const lhs = AllocatedRectangularCuboid({10, 20, 30}, {100, 200, 300});
    SECTION("all corners with distance 0") {
        auto const rhs = AllocatedRectangularCuboid({10, 20, 30}, {100, 200, 300});
        auto const actual = Anchor::value_of(lhs, rhs);
        auto const expected = decltype(actual){0, Corner::BACK_RIGHT_BOTTOM};
        CHECK(actual == expected);
    }
    SECTION("front corners with distance 0") {
        auto const rhs = AllocatedRectangularCuboid({5, 20, 30}, {105, 200, 300});
        auto const actual = Anchor::value_of(lhs, rhs);
        auto const expected = decltype(actual){0, Corner::FRONT_RIGHT_BOTTOM};
        CHECK(actual == expected);
    }
    SECTION("left corners with distance 0") {
        auto const rhs = AllocatedRectangularCuboid({10, 15, 30}, {100, 205, 300});
        auto const actual = Anchor::value_of(lhs, rhs);
        auto const expected = decltype(actual){0, Corner::BACK_LEFT_BOTTOM};
        CHECK(actual == expected);
    }
    SECTION("top corners with distance 0") {
        auto const rhs = AllocatedRectangularCuboid({10, 20, 25}, {100, 200, 305});
        auto const actual = Anchor::value_of(lhs, rhs);
        auto const expected = decltype(actual){0, Corner::BACK_RIGHT_TOP};
        CHECK(actual == expected);
    }
}

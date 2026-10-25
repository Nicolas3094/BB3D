#include <catch.hpp>

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/Anchor.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

TEST_CASE("direction to corner", "[Directions][Geometry]") {
    SECTION("xyz") {
        auto const directions = Directions{.x = true, .y = true, .z = true};
        CHECK(directions.to_corner() == Corner::FRONT_LEFT_TOP);
    }
    SECTION("yz") {
        auto const directions = Directions{.x = false, .y = true, .z = true};
        CHECK(directions.to_corner() == Corner::BACK_LEFT_TOP);
    }
    SECTION("xz") {
        auto const directions = Directions{.x = true, .y = false, .z = true};
        CHECK(directions.to_corner() == Corner::FRONT_RIGHT_TOP);
    }
    SECTION("xy") {
        auto const directions = Directions{.x = true, .y = true, .z = false};
        CHECK(directions.to_corner() == Corner::FRONT_LEFT_BOTTOM);
    }
    SECTION("x") {
        auto const directions = Directions{.x = true, .y = false, .z = false};
        CHECK(directions.to_corner() == Corner::FRONT_RIGHT_BOTTOM);
    }
    SECTION("y") {
        auto const directions = Directions{.x = false, .y = true, .z = false};
        CHECK(directions.to_corner() == Corner::BACK_LEFT_BOTTOM);
    }
    SECTION("z") {
        auto const directions = Directions{.x = false, .y = false, .z = true};
        CHECK(directions.to_corner() == Corner::BACK_RIGHT_TOP);
    }
    SECTION("none") {
        auto const directions = Directions{.x = false, .y = false, .z = false};
        CHECK(directions.to_corner() == Corner::BACK_RIGHT_BOTTOM);
    }
}

TEST_CASE("Directions constructor", "[Directions][Geometry]") {
    auto const directions = Directions();
    CHECK(directions.x == false);
    CHECK(directions.y == false);
    CHECK(directions.z == false);
}

TEST_CASE("corner_position", "[Anchor][Geometry]") {
    auto const measurement = DiscreteVector{1, 2, 3};
    auto const initial_position = DiscreteVector{5, 7, 11};
    auto const allocated_rectangular_cuboid = AllocatedRectangularCuboid(measurement, initial_position);
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::BACK_RIGHT_BOTTOM) == DiscreteVector{5, 7, 11});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::FRONT_RIGHT_BOTTOM) == DiscreteVector{6, 7, 11});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::BACK_LEFT_BOTTOM) == DiscreteVector{5, 9, 11});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::FRONT_LEFT_BOTTOM) == DiscreteVector{6, 9, 11});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::BACK_RIGHT_TOP) == DiscreteVector{5, 7, 14});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::FRONT_RIGHT_TOP) == DiscreteVector{6, 7, 14});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::BACK_LEFT_TOP) == DiscreteVector{5, 9, 14});
    CHECK(Anchor::corner_position(allocated_rectangular_cuboid, Corner::FRONT_LEFT_TOP) == DiscreteVector{6, 9, 14});
}

TEST_CASE("position_at_corner", "[Anchor][Geometry]") {
    auto const small_object = RectangularCuboid({3, 5, 7});
    auto const large_object = AllocatedRectangularCuboid({23, 29, 31}, {11, 13, 19});
    SECTION("BACK_RIGHT_BOTTOM") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::BACK_RIGHT_BOTTOM);
        auto const expected = DiscreteVector{11, 13, 19};
        CHECK(actual == expected);
    }
    SECTION("FRONT_RIGHT_BOTTOM") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::FRONT_RIGHT_BOTTOM);
        auto const expected = DiscreteVector{31, 13, 19};
        CHECK(actual == expected);
    }
    SECTION("BACK_LEFT_BOTTOM") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::BACK_LEFT_BOTTOM);
        auto const expected = DiscreteVector{11, 37, 19};
        CHECK(actual == expected);
    }
    SECTION("FRONT_LEFT_BOTTOM") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::FRONT_LEFT_BOTTOM);
        auto const expected = DiscreteVector{31, 37, 19};
        CHECK(actual == expected);
    }
    SECTION("BACK_RIGHT_TOP") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::BACK_RIGHT_TOP);
        auto const expected = DiscreteVector{11, 13, 43};
        CHECK(actual == expected);
    }
    SECTION("FRONT_RIGHT_TOP") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::FRONT_RIGHT_TOP);
        auto const expected = DiscreteVector{31, 13, 43};
        CHECK(actual == expected);
    }
    SECTION("BACK_LEFT_TOP") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::BACK_LEFT_TOP);
        auto const expected = DiscreteVector{11, 37, 43};
        CHECK(actual == expected);
    }
    SECTION("FRONT_LEFT_TOP") {
        auto const actual = Anchor::position_at_corner(small_object, large_object, Corner::FRONT_LEFT_TOP);
        auto const expected = DiscreteVector{31, 37, 43};
        CHECK(actual == expected);
    }
#ifdef SCLP_DEBUG
    SECTION("failure") {
        auto const large_object = AllocatedRectangularCuboid({1, 2, 3}, {11, 13, 19});
        CHECK_THROWS(Anchor::position_at_corner(small_object, large_object, Corner::BACK_LEFT_BOTTOM));
    }
#endif
}

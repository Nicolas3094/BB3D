#include <catch.hpp>
#include <optional>

#include "sclp/Geometry/Area.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

TEST_CASE("constructor from measurement", "[Area][Geometry]") {
    auto const initial_position = DiscreteVector2D{2, 3};
    auto const measurement = DiscreteVector2D{5, 7};
    auto const plane = Plane::XY;
    auto const orthogonal_position = DiscreteVector2D::scalar(11);
    auto const area = Area::from_measurement(plane, measurement, initial_position, orthogonal_position);
    CHECK(area.plane() == plane);
    CHECK(area.initial_position() == initial_position);
    CHECK(area.final_position() == DiscreteVector2D{7, 10});
    CHECK(area.measurement() == measurement);
    CHECK(area.orthogonal_position() == orthogonal_position);
    CHECK(area.prod() == 35);
    CHECK(area == area);
}

TEST_CASE("constructor from positions", "[Area][Geometry]") {
    auto const initial_position = DiscreteVector2D{2, 3};
    auto const final_position = DiscreteVector2D{5, 7};
    auto const plane = Plane::YZ;
    auto const orthogonal_position = DiscreteVector2D::scalar(11);
    auto const area = Area::from_positions(plane, initial_position, final_position, orthogonal_position);
    CHECK(area.plane() == plane);
    CHECK(area.initial_position() == initial_position);
    CHECK(area.final_position() == final_position);
    CHECK(area.measurement() == DiscreteVector2D{3, 4});
    CHECK(area.orthogonal_position() == orthogonal_position);
    CHECK(area.prod() == 12);
    CHECK(area == area);
}

TEST_CASE("overlaps", "[Area][Geometry]") {
    // perpendicular position is not relevant for overlap
    auto const plane = Plane::YZ;
    auto const area = Area::from_measurement(plane, {3, 5}, {30, 50}, 44);
    SECTION("true") {
        auto const others = std::vector<Area>{
            Area::from_measurement(plane, {3, 5}, {30, 50}, 21),
            Area::from_measurement(plane, {1, 3}, {31, 51}, 82),
            Area::from_measurement(plane, {5, 7}, {29, 49}, 10),
            Area::from_measurement(plane, {3, 5}, {29, 50}, 15),
            Area::from_measurement(plane, {3, 5}, {30, 49}, 43),
            Area::from_measurement(plane, {3, 5}, {29, 49}, 60),
        };
        for (auto const & other : others) {
            CHECK(area.overlaps(other));
            CHECK(other.overlaps(area));
        }
    }
    SECTION("false") {
        auto const others = std::vector<Area>{
            Area::from_measurement(plane, {3, 5}, {33, 50}, 49),
            Area::from_measurement(plane, {3, 5}, {30, 55}, 11),
            Area::from_measurement(plane, {3, 5}, {33, 55}, 69),
            Area::from_measurement(plane, {3, 5}, {300, 500}, 67),
            Area::from_measurement(plane, {3, 5}, {0, 0}, 29),
        };
        for (auto const & other : others) {
            CHECK_FALSE(area.overlaps(other));
            CHECK_FALSE(other.overlaps(area));
        }
    }
    SECTION("throws for wrong plane") {
        CHECK_THROWS(area.overlaps(Area::from_measurement(Plane::XY, {3, 5}, {30, 50}, 44)));
        CHECK_THROWS(area.overlaps(Area::from_measurement(Plane::XZ, {3, 5}, {30, 50}, 44)));
    }
}

TEST_CASE("contains", "[Area][Geometry]") {
    // perpendicular position is not relevant for contains
    auto const plane = Plane::XY;
    auto const big = Area::from_measurement(plane, {10, 10}, {0, 0}, 56);
    auto const small = Area::from_measurement(plane, {1, 1}, {0, 0}, 25);
    auto const small_with_offset = Area::from_measurement(plane, {1, 1}, {4, 4}, 23);
    auto const outside = Area::from_measurement(plane, {2, 2}, {0, 9}, 55);
    auto const will_throw = Area::from_measurement(Plane::YZ, {3, 5}, {30, 50}, 44);
    CHECK(big.contains(big));
    CHECK(big.contains(small));
    CHECK(big.contains(small_with_offset));
    CHECK_FALSE(small.contains(outside));
    CHECK_FALSE(big.contains(outside));
    CHECK_THROWS(big.contains(will_throw));
}

TEST_CASE("overlap_with", "[Area][Geometry]") {
    // perpendicular position is not relevant for overlap_with
    auto const plane = Plane::YZ;
    auto const orthogonal_position = DiscreteVector2D::scalar(44);
    auto const area = Area::from_measurement(plane, {3, 5}, {30, 50}, orthogonal_position);
    SECTION("true") {
        SECTION("case 1") {
            auto const other = Area::from_measurement(plane, {3, 5}, {30, 50}, 21);
            auto const expected = area;
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 2") {
            auto const other = Area::from_measurement(plane, {1, 3}, {31, 51}, 82);
            auto const expected = Area::from_measurement(plane, {1, 3}, {31, 51}, orthogonal_position);
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 3") {
            auto const other = Area::from_measurement(plane, {5, 7}, {29, 49}, 10);
            auto const expected = area;
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 4") {
            auto const other = Area::from_measurement(plane, {2, 4}, {29, 49}, 15);
            auto const expected = Area::from_measurement(plane, {1, 3}, {30, 50}, orthogonal_position);
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 5") {
            auto const other = Area::from_measurement(plane, {4, 4}, {31, 49}, 15);
            auto const expected = Area::from_measurement(plane, {2, 3}, {31, 50}, orthogonal_position);
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 6") {
            auto const other = Area::from_measurement(plane, {2, 4}, {29, 51}, 15);
            auto const expected = Area::from_measurement(plane, {1, 4}, {30, 51}, orthogonal_position);
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
        SECTION("case 7") {
            auto const other = Area::from_measurement(plane, {2, 4}, {31, 51}, 15);
            auto const expected = Area::from_measurement(plane, {2, 4}, {31, 51}, orthogonal_position);
            CHECK(area.overlaping_area_with(other).value() == expected);
        }
    }
    SECTION("false") {
        SECTION("case 1") {
            auto const other = Area::from_measurement(plane, {3, 5}, {33, 50}, 49);
            auto const expected = std::nullopt;
            CHECK(area.overlaping_area_with(other) == expected);
        }
        SECTION("case 2") {
            auto const other = Area::from_measurement(plane, {3, 5}, {30, 55}, 49);
            auto const expected = std::nullopt;
            CHECK(area.overlaping_area_with(other) == expected);
        }
        SECTION("case 3") {
            auto const other = Area::from_measurement(plane, {3, 5}, {100, 200}, 49);
            auto const expected = std::nullopt;
            CHECK(area.overlaping_area_with(other) == expected);
        }
    }
}

#include <catch.hpp>

#include <vector>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"

using namespace sclp;

TEST_CASE("constructor", "[RectangularCuboid][Geometry]") {
    auto const x = CoordinateType{1};
    auto const y = CoordinateType{2};
    auto const z = CoordinateType{3};
    auto const measurement = DiscreteVector{x, y, z};
    auto const rectangular_cuboid = RectangularCuboid(measurement);
    CHECK(rectangular_cuboid.measurement().x() == x);
    CHECK(rectangular_cuboid.measurement().y() == y);
    CHECK(rectangular_cuboid.measurement().z() == z);
}

TEST_CASE("equality", "[RectangularCuboid][Geometry]") {
    auto const original = RectangularCuboid({1, 2, 3});
    auto others = std::vector<RectangularCuboid>();
    others.emplace_back(DiscreteVector{1, 2, 3});
    others.emplace_back(DiscreteVector{9, 2, 3});
    others.emplace_back(DiscreteVector{1, 9, 3});
    others.emplace_back(DiscreteVector{1, 2, 9});
    SECTION("operator==") {
        SECTION("true") {
            CHECK(original == original);
            CHECK(original == others[0]);
        }
        SECTION("false") {
            for (auto i = 0u; i < others.size(); ++i) {
                for (auto j = i + 1; j < others.size(); ++j) {
                    CHECK_FALSE(others[i] == others[j]);
                }
            }
        }
    }
}

TEST_CASE("surface_area", "[RectangularCuboid][Geometry]") {
    auto const measurement = DiscreteVector{2, 3, 5};
    auto const rectangular_cuboid = RectangularCuboid(measurement);
    CHECK(rectangular_cuboid.surface_area() == 62);
}

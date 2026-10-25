#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Packing/SmallItem.hpp"

using namespace sclp;

TEST_CASE("constructor", "[SmallItem][Packing]") {
    auto const x = CoordinateType{1};
    auto const y = CoordinateType{2};
    auto const z = CoordinateType{3};
    auto const index = Index{8};
    auto const quantity = Quantity{4};
    auto const measurement = DiscreteVector{x, y, z};
    auto const orientation = EnabledOrientations({
        OrientationType::XYZ,
        OrientationType::ZYX,
        OrientationType::ZXY,
    });
    auto const item = SmallItem{index, quantity, measurement, orientation};
    CHECK(item.index() == index);
    CHECK(item.quantity() == quantity);
    CHECK(item.measurement().x() == x);
    CHECK(item.measurement().y() == y);
    CHECK(item.measurement().z() == z);
    CHECK(item.orientation()[OrientationType::XYZ]);
    CHECK_FALSE(item.orientation()[OrientationType::YXZ]);
    CHECK(item.orientation()[OrientationType::ZYX]);
    CHECK_FALSE(item.orientation()[OrientationType::XZY]);
    CHECK(item.orientation()[OrientationType::ZXY]);
    CHECK_FALSE(item.orientation()[OrientationType::YZX]);
}

TEST_CASE("equality operator", "[SmallItem][Packing]") {
    auto const orientation = EnabledOrientations({
        OrientationType::XYZ,
        OrientationType::ZYX,
        OrientationType::ZXY,
    });
    auto const item = SmallItem{0, 4, {1, 2, 3}, orientation};
    SECTION("equal") {
        auto const other = SmallItem{0, 4, {1, 2, 3}, orientation};
        CHECK(item == other);
    }
    SECTION("different measurement") {
        auto const other = SmallItem{0, 4, {9, 2, 3}, orientation};
        CHECK_FALSE((item == other));
    }
    SECTION("different quantity") {
        auto const other = SmallItem{0, 9, {1, 2, 3}, orientation};
        CHECK_FALSE((item == other));
    }
    SECTION("different index") {
        auto const other = SmallItem{1, 4, {1, 2, 3}, orientation};
        CHECK_FALSE((item == other));
    }
    SECTION("different orientation") {
        auto const other = SmallItem{0, 4, {1, 2, 3}, EnabledOrientations::all()};
        CHECK_FALSE(item == other);
    }
}

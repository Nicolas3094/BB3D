#include <catch.hpp>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Packing/SmallItem.hpp"
#include "sclp/Packing/AllocatedSmallItem.hpp"


using namespace sclp;

TEST_CASE("no orientation", "[AllocatedSmallItem][Packing]") {
    auto const index = Index{0};
    auto const quantity = Quantity{4};
    auto const measurement = DiscreteVector{1, 2, 3};
    auto const small_item = SmallItem(index, quantity, measurement);
    auto const orientation = OrientationType::ZXY;
    auto const initial_position = DiscreteVector{4, 5, 6};
    auto const final_position = DiscreteVector{7, 6, 8};
    auto const allocated_item = AllocatedSmallItem{small_item, initial_position, orientation};
    CHECK(allocated_item.type() == small_item);
    CHECK(allocated_item.initial_position() == initial_position);
    CHECK(allocated_item.final_position() == final_position);
    CHECK(allocated_item.measurement() == rotate(measurement, orientation));
    CHECK(allocated_item == allocated_item);
}


TEST_CASE("with orientation", "[AllocatedSmallItem]") {
    auto const index = Index{0};
    auto const quantity = Quantity{4};
    auto const measurement = DiscreteVector{1, 2, 3};
    auto const orientation = {OrientationType::XYZ, OrientationType::ZXY};
    auto const small_item = SmallItem(index, quantity, measurement, orientation);
    const auto initial_position = DiscreteVector{5, 6, 7};
    SECTION("XYZ") {
        const auto orientation = OrientationType::XYZ;
        const auto allocated_small_item = AllocatedSmallItem{small_item, initial_position, orientation};
        SECTION("type") {
            CHECK(allocated_small_item.type().measurement() == small_item.measurement());
            CHECK(allocated_small_item.type().quantity() == small_item.quantity());
        }
        SECTION("measurement and positions") {
            const auto oriented_measurement = sclp::rotate(small_item.measurement(), orientation);
            CHECK(allocated_small_item.measurement() == oriented_measurement);
            CHECK(allocated_small_item.initial_position() == initial_position);
            CHECK(allocated_small_item.final_position() == initial_position + DiscreteVector{oriented_measurement});
        }
        SECTION("orientation") {
            CHECK(allocated_small_item.orientation() == orientation);
        }
        SECTION("equality") {
            CHECK(allocated_small_item == allocated_small_item);
        }
    }
    SECTION("ZXY") {
        const auto orientation = OrientationType::ZXY;
        const auto allocated_small_item = AllocatedSmallItem{small_item, initial_position, orientation};
        SECTION("type") {
            CHECK(allocated_small_item.type().measurement() == small_item.measurement());
            CHECK(allocated_small_item.type().quantity() == small_item.quantity());
        }
        SECTION("measurement and positions") {
            const auto oriented_measurement = sclp::rotate(small_item.measurement(), orientation);
            CHECK(allocated_small_item.measurement() == oriented_measurement);
            CHECK(allocated_small_item.initial_position() == initial_position);
            CHECK(allocated_small_item.final_position() == initial_position + DiscreteVector{oriented_measurement});
        }
        SECTION("orientation") {
            CHECK(allocated_small_item.orientation() == orientation);
        }
        SECTION("equality") {
            CHECK(allocated_small_item == allocated_small_item);
        }
    }
}

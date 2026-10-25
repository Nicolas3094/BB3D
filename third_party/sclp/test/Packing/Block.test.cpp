#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Packing/Block.hpp"

using namespace sclp;

auto make_small_item(DiscreteVector measurement, Quantity quantity, Index index = 0) {
    return SmallItem{index, quantity, measurement, EnabledOrientations::all()};
}

TEST_CASE("empty block", "[Block]") {
    auto const block = Block();
    CHECK(block.allocated_small_items().size() == 0);
    CHECK(block.measurement() == DiscreteVector::zeros());
    CHECK(block.volume() == 0);
    CHECK(block.quantity_map() == QuantityMap{});
    CHECK(block.valid_quantity());
    CHECK(block.occupied_volume_ratio() == Catch::Approx(0.0));
}

TEST_CASE("single item in a block", "[Block]") {
    auto const & small_item = make_small_item(DiscreteVector{2, 5, 11}, 2);
    SECTION("XYZ") {
        auto block = Block();
        block.add(small_item, DiscreteVector::zeros(), OrientationType::XYZ);
        CHECK(block.allocated_small_items().size() == 1);
        CHECK(block.measurement() == small_item.measurement());
        CHECK(block.volume() == small_item.measurement().prod());
        CHECK(block.quantity_map() == QuantityMap{{&small_item, 1}});
        CHECK(block.valid_quantity());
        CHECK(block.occupied_volume_ratio() == Catch::Approx(1.0));
    }
    SECTION("ZYX") {
        auto block = Block();
        block.add(small_item, DiscreteVector::zeros(), OrientationType::ZYX);
        CHECK(block.allocated_small_items().size() == 1);
        CHECK(block.measurement() == DiscreteVector{11, 5, 2});
        CHECK(block.volume() == Volume(11 * 5 * 2));
        CHECK(block.quantity_map() == QuantityMap{{&small_item, 1}});
        CHECK(block.valid_quantity());
        CHECK(block.occupied_volume_ratio() == Catch::Approx(1.0));
    }
}

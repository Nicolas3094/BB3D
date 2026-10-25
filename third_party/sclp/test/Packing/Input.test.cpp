#include <catch.hpp>

#include <vector>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/LargeObject.hpp"
#include "sclp/Packing/SmallItem.hpp"

using namespace sclp;

TEST_CASE("constructor with no item", "[Input][Packing]") {
    auto const large_object_measurements = DiscreteVector{11, 13, 17};
    auto const input = Input(
        LargeObject{large_object_measurements},
        std::vector<SmallItem>{});
    SECTION("large object") {
        auto expected_large_object = LargeObject{large_object_measurements};
        CHECK(input.large_object() == expected_large_object);
    }
    SECTION("small items") {
        CHECK(input.small_items().empty());
    }
    SECTION("aggregate properties") {
        CHECK(input.quantity_map() == QuantityMap{});
        CHECK(input.total_quantity() == 0);
        CHECK(input.total_volume() == 0);
    }
}

TEST_CASE("constructor", "[Input][Packing]") {
    auto const large_object_measurements = DiscreteVector{11, 13, 17};
    auto const input = Input(
        LargeObject{large_object_measurements},
        std::vector<SmallItem>{
            {0, 41, DiscreteVector{3, 5, 7}},
            {1, 97, DiscreteVector{11, 13, 19}},
        });
    SECTION("large object") {
        auto expected_large_object = LargeObject{large_object_measurements};
        CHECK(input.large_object() == expected_large_object);
    }
    SECTION("small items") {
        auto expected_small_items = std::vector<SmallItem>();
        expected_small_items.emplace_back(0, 41, DiscreteVector{3, 5, 7});
        expected_small_items.emplace_back(1, 97, DiscreteVector{11, 13, 19});
        for (auto i = 0u; i < input.small_items().size(); ++i) {
            CHECK(input.small_items()[i] == expected_small_items[i]);
        }
    }
    SECTION("aggregate properties") {
        auto const expected_quantity_map = QuantityMap{
            {&input.small_items()[0], 41},
            {&input.small_items()[1], 97},
        };
        CHECK(input.quantity_map() == expected_quantity_map);
        CHECK(input.total_quantity() == 138);
        CHECK(input.total_volume() == 267854);
    }
}

TEST_CASE("constructor with invalid indices", "[Input][Packing]") {
    auto const large_object_measurements = DiscreteVector{11, 13, 17};
    CHECK_THROWS(Input(
        LargeObject{large_object_measurements},
        std::vector<SmallItem>{
            {0, 1, DiscreteVector{1, 1, 1}},
            {2, 2, DiscreteVector{2, 2, 2}},
            // invalid, indices must go from 0 to N-1
        }));
}

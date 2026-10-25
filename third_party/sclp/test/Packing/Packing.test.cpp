#include <catch.hpp>

#include <vector>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Packing/SmallItem.hpp"

using namespace sclp;

TEST_CASE("constructor", "[Packing][Packing]") {
    auto const small_items = std::vector<SmallItem>{
        {0, 83, {1, 2, 3}},
        {1, 61, {4, 5, 6}},
        {2, 23, {7, 8, 9}},
    };
    auto packing = Packing{};
    packing.add(small_items[0], DiscreteVector{10, 11, 12}, OrientationType::XYZ);
    packing.add(small_items[1], DiscreteVector{13, 14, 15}, OrientationType::XYZ);
    packing.add(small_items[2], DiscreteVector{16, 17, 18}, OrientationType::XYZ);
    packing.add(small_items[2], DiscreteVector{19, 20, 21}, OrientationType::XYZ);
    SECTION("allocated small item 0") {
        CHECK(packing.allocated_small_items()[0].type() == small_items[0]);
        CHECK(packing.allocated_small_items()[0].initial_position() == DiscreteVector{10, 11, 12});
        CHECK(packing.allocated_small_items()[0].final_position() == DiscreteVector{11, 13, 15});
    }
    SECTION("allocated small item 1") {
        CHECK(packing.allocated_small_items()[1].type() == small_items[1]);
        CHECK(packing.allocated_small_items()[1].initial_position() == DiscreteVector{13, 14, 15});
        CHECK(packing.allocated_small_items()[1].final_position() == DiscreteVector{17, 19, 21});
    }
    SECTION("aggregate properties") {
        CHECK(packing.allocated_small_items().size() == 4);
        CHECK(packing.allocated_volume() == 1134);
    }
    SECTION("quantity_map") {
        const auto expected_quantity_map = QuantityMap{
            {&small_items[0], Quantity{1}},
            {&small_items[1], Quantity{1}},
            {&small_items[2], Quantity{2}},
        };
        CHECK(packing.quantity_map() == expected_quantity_map);
    }
}

TEST_CASE("basic instantiation", "[Packing][Packing]") {
    const auto small_items = std::vector<SmallItem>{
        SmallItem(0, 100, {1, 2, 3}, EnabledOrientations::xyz()),
        SmallItem(1, 200, {7, 11, 5}, EnabledOrientations::all()),
    };
    auto packing = std::make_shared<Packing>();
    packing->add(small_items[0], {0, 0, 0}, OrientationType::XYZ);
    packing->add(small_items[1], {1, 0, 0}, OrientationType::ZXY);
    const auto expected_allocated_small_items = std::vector<AllocatedSmallItem>{
        {small_items[0], {0, 0, 0}, OrientationType::XYZ},
        {small_items[1], {1, 0, 0}, OrientationType::ZXY},
    };
    const auto expected_quantity_map = QuantityMap{
        {&small_items[0], Quantity{1}},
        {&small_items[1], Quantity{1}},
    };
    CHECK((packing->allocated_small_items() == expected_allocated_small_items));
    CHECK(packing->measurement() == DiscreteVector{6, 7, 11});
    CHECK(packing->quantity_map() == expected_quantity_map);
    CHECK(packing->valid_quantity());
    CHECK(packing->volume() == 462);
    CHECK(packing->allocated_volume() == 391);
    CHECK(packing->occupied_volume_ratio() == Catch::Approx(0.8463203463203464));
}

TEST_CASE("invalid quantity", "[Packing][Packing]") {
    const auto small_items = std::vector<SmallItem>{
        SmallItem(0, 1, {1, 2, 3}, EnabledOrientations::xyz()),
    };
    auto packing = std::make_shared<Packing>();
    packing->add(small_items[0], {0, 0, 0}, OrientationType::XYZ);
    packing->add(small_items[0], {1, 0, 0}, OrientationType::XYZ);
    CHECK_FALSE(packing->valid_quantity());
}

TEST_CASE("add empty packings", "[Packing][Packing]") {
    auto packing = std::vector{
        std::make_shared<Packing>(),
        std::make_shared<Packing>(),
        std::make_shared<Packing>(),
    };
    packing[1]->add(packing[0], DiscreteVector::zeros());
    CHECK_THROWS(packing[2]->add(packing[1], DiscreteVector::zeros()));
}

TEST_CASE("hash", "[Packing][Packing]") {
    // the hash of a packing is defined by
    // its measurement and the quantity of each item that composes it
    const auto small_items = std::vector<SmallItem>{
        SmallItem(0, 100, {1, 2, 3}, EnabledOrientations::xyz()),
        SmallItem(1, 200, {7, 11, 5}, EnabledOrientations::all()),
    };
    auto first_packing = std::make_shared<Packing>();
    first_packing->add(small_items[0], {0, 0, 0}, OrientationType::XYZ);
    first_packing->add(small_items[0], {1, 0, 0}, OrientationType::XYZ);
    first_packing->add(small_items[1], {9, 25, 33}, OrientationType::YZX);
    first_packing->update_hash();
    auto second_packing = std::make_shared<Packing>();
    second_packing->add(small_items[1], {0, 0, 0}, OrientationType::ZXY);
    second_packing->add(small_items[0], {6, 0, 0}, OrientationType::XYZ);
    second_packing->add(small_items[0], {19, 28, 37}, OrientationType::XYZ);
    second_packing->update_hash();
    CHECK(first_packing->hash() == second_packing->hash());
    SECTION("std operators") {
        SECTION("hash with Sptr") {
            auto hash_ptr = std::hash<Packing::Sptr>();
            CHECK(hash_ptr(first_packing) == hash_ptr(second_packing));
        }
        SECTION("hash with object") {
            auto hash = std::hash<Packing>();
            CHECK(hash(*first_packing) == hash(*second_packing));
        }
        SECTION("equal_to with Sptr") {
            auto equal_ptr = std::equal_to<Packing::Sptr>();
            CHECK(equal_ptr(first_packing, second_packing));
        }
        SECTION("equal_to with object") {
            auto equal = std::equal_to<Packing>();
            CHECK(equal(*first_packing, *second_packing));
        }
    }
}

TEST_CASE("packing inside packing", "[Packing][Packing]") {
    const auto small_items = std::vector<SmallItem>{
        SmallItem(0, 100, {1, 2, 3}, EnabledOrientations::all()),
        SmallItem(1, 200, {7, 11, 5}, EnabledOrientations::all()),
    };
    auto large_internal_packing = std::make_shared<Packing>();
    large_internal_packing->add(small_items[0], {0, 0, 0}, OrientationType::XYZ);
    large_internal_packing->add(small_items[1], {1, 0, 0}, OrientationType::ZXY);
    auto small_internal_packing = std::make_shared<Packing>();
    small_internal_packing->add(small_items[0], {0, 0, 0}, OrientationType::ZXY);
    auto external_packing = std::make_shared<Packing>();
    external_packing->add(small_internal_packing, {0, 0, 0});
    external_packing->add(large_internal_packing, {10, 20, 30});
    external_packing->build();
    const auto expected_allocated_small_items = std::vector<AllocatedSmallItem>{
        {small_items[0], {0, 0, 0}, OrientationType::ZXY},
        {small_items[0], {10, 20, 30}, OrientationType::XYZ},
        {small_items[1], {11, 20, 30}, OrientationType::ZXY},
    };
    const auto expected_quantity_map = QuantityMap{
        {&small_items[0], Quantity{2}},
        {&small_items[1], Quantity{1}},
    };
    CHECK((external_packing->allocated_small_items() == expected_allocated_small_items));
    CHECK(external_packing->measurement() == DiscreteVector{16, 27, 41});
    CHECK(external_packing->quantity_map() == expected_quantity_map);
    CHECK(external_packing->valid_quantity());
    CHECK(external_packing->volume() == 17712);
    CHECK(external_packing->allocated_volume() == 397);
    CHECK(external_packing->occupied_volume_ratio() == Catch::Approx(0.022414182475158084));
}

TEST_CASE("throw when getting items without building", "[Packing][Packing]") {
    const auto small_items = std::vector<SmallItem>{
        SmallItem(0, 100, {1, 2, 3}, EnabledOrientations::all()),
    };
    auto small_internal_packing = std::make_shared<Packing>();
    small_internal_packing->add(small_items[0], {0, 0, 0}, OrientationType::ZXY);
    auto external_packing = std::make_shared<Packing>();
    external_packing->add(small_internal_packing, {0, 0, 0});
    CHECK_THROWS(external_packing->allocated_small_items());
}

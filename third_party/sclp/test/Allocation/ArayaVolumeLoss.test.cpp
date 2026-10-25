#include <catch.hpp>

#include <filesystem>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/ArayaVolumeLoss.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types/Measure.hpp"

auto const data_dir = std::filesystem::path("test/data/ArayaVolumeLoss/");

using namespace sclp;

// ---------- get_small_item_measurement ----------
// Notice that its behavior does not match the behavior one actually expects
//
// Here, one wants to show that the function does not return
// the values one expects.

TEST_CASE("get_small_item_measurement - set 0 - instance 1 (0) - compare with Araya's code", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "00.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            // Araya's and one's code:
            {Axis::X, {{Axis::X, 108}, {Axis::Y, 76}, {Axis::Z, 30}}},
            {Axis::Y, {{Axis::X, 0}, {Axis::Y, 0}, {Axis::Z, 0}}},
            {Axis::Z, {{Axis::X, 0}, {Axis::Y, 0}, {Axis::Z, 0}}},
            // // expected:
            // {Axis::X, {{Axis::X, 108}, {Axis::Y, 108}, {Axis::Z, 0}}},
            // {Axis::Y, {{Axis::X, 76}, {Axis::Y, 76}, {Axis::Z, 0}}},
            // {Axis::Z, {{Axis::X, 0}, {Axis::Y, 0}, {Axis::Z, 30}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ArayaVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

TEST_CASE("get_small_item_measurement - set 0 - instance 2 (1) - compare with Araya's code", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "01.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            // Araya's and one's code:
            {Axis::X, {{Axis::X, 49}, {Axis::Y, 25}, {Axis::Z, 21}}},
            {Axis::Y, {{Axis::X, 0}, {Axis::Y, 0}, {Axis::Z, 0}}},
            {Axis::Z, {{Axis::X, 21}, {Axis::Y, 49}, {Axis::Z, 25}}},
            // // expected:
            // {Axis::X, {{Axis::X, 49}, {Axis::Y, 49}, {Axis::Z, 0}}},
            // {Axis::Y, {{Axis::X, 25}, {Axis::Y, 25}, {Axis::Z, 25}}},
            // {Axis::Z, {{Axis::X, 21}, {Axis::Y, 21}, {Axis::Z, 21}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ArayaVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

TEST_CASE("get_small_item_measurement - set 0 - instance 6 (5) - compare with Araya's code", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "02.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            // Araya's and one's code:
            {Axis::X, {{Axis::X, 86}, {Axis::Y, 84}, {Axis::Z, 45}}},
            {Axis::Y, {{Axis::X, 84}, {Axis::Y, 45}, {Axis::Z, 86}}},
            {Axis::Z, {{Axis::X, 45}, {Axis::Y, 86}, {Axis::Z, 84}}},
            // // expected:
            // {Axis::X, {{Axis::X, 86}, {Axis::Y, 86}, {Axis::Z, 86}}},
            // {Axis::Y, {{Axis::X, 84}, {Axis::Y, 84}, {Axis::Z, 84}}},
            // {Axis::Z, {{Axis::X, 45}, {Axis::Y, 45}, {Axis::Z, 45}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ArayaVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

// ---------- compute_measurement_waste ----------
//
// Here, one wants to show that the function does not
// take the number of available items into account,
// so if there is only one or the are many items left,
// it makes no difference.

TEST_CASE("one item with one orientation", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "10.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const volume_loss = ArayaVolumeLoss(
        input.large_object().measurement(),
        input.quantity_map());
    auto singular_block = Block();
    singular_block.add(
        item,
        DiscreteVector::zeros(),
        OrientationType::XYZ);
    SECTION("only the item of the block remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 1}, // 1 of the block
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == input.large_object().measurement().x());
    }
    SECTION("one extra item remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 2}, // 1 of the block, 1 remaining
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 7); // 7 and not 100 % 17 = 5 because get_small_item_measurement is "wrong"
    }
    SECTION("all items remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, item.quantity() - 1}, // 1 of the block, the others remaining
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 7); // 7 and not 100 % 17 = 5 because get_small_item_measurement is "wrong"
    }
}

TEST_CASE("one item with all orientation", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "20.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const volume_loss = ArayaVolumeLoss(
        input.large_object().measurement(),
        input.quantity_map());
    auto singular_block = Block();
    singular_block.add(
        item,
        DiscreteVector::zeros(),
        OrientationType::XYZ);
    SECTION("only the item of the block remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 1}, // 1 of the block
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == input.large_object().measurement().x());
    }
    SECTION("one extra item remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 2}, // 1 of the block, 1 remaining
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 0);
    }
    SECTION("all items remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, item.quantity() - 1}, // 1 of the block, the others remaining
        };
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 0);
    }
}

// ---------- compute_measurement_waste ----------
//
// Here, one wants to show that the order in which the
// items are in the input matter.
// Consider the following example:
//
// 0. small_items_to_reach_measurement (simplified) when the items of the input are [i_0, i_1]:
//  17    23    34    40    46
//  |     |     |     |     |
//  |     |     |     |     | i_1
//  |     |     |     | i_1
//  |     |     | i_0
//  |     | i_1
//  | i_0
//
// 1. small_items_to_reach_measurement (simplified) when the items of the input are [i_1, i_0]:
//  17    23    34    40    46
//  |     |     |     |     |
//  |     |     |     |     | i_1
//  |     |     |     | i_0
//  |     |     | i_0
//  |     | i_1
//  | i_0
//
// 40 is the measurement for which they differ.
//
// - If the block is made of i_0, i.e. only i_1 is available,
//   the maximum reachable measurement (and measurement waste)
//   for a measurement limit of 50 is:
//     0. 46 (4)
//     1. 46 (4)
// - If the block is made of i_1, i.e. only i_0 is available,
//   the maximum reachable measurement (and measurement waste)
//   for a measurement limit of 50 is:
//     0. 34 (16)
//     1. 40 (10)
//
// Therefore, one notices a difference in the last case.


TEST_CASE("compute_measurement_waste - original order", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "30.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item_0 = input.small_items()[0];
    auto const & item_1 = input.small_items()[1];
    auto const volume_loss = ArayaVolumeLoss(
        input.large_object().measurement(),
        input.quantity_map());
    auto singular_block = Block();
    auto const remaining_small_items_quantity = QuantityMap{
        {&item_0, 1},
        {&item_1, 1},
    };
    SECTION("case 1 - block made of item_1") {
        singular_block.add(
            item_1,
            DiscreteVector::zeros(),
            OrientationType::XYZ);
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 16);
    }
    SECTION("case 2 - block made of item_0") {
        singular_block.add(
            item_0,
            DiscreteVector::zeros(),
            OrientationType::XYZ);
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 4);
    }
}

TEST_CASE("compute_measurement_waste - swap items", "[ArayaVolumeLoss]") {
    auto const file_path = data_dir / "31.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    // the order has been changed so they match the items of the previous test
    auto const & item_1 = input.small_items()[0];
    auto const & item_0 = input.small_items()[1];
    auto const volume_loss = ArayaVolumeLoss(
        input.large_object().measurement(),
        input.quantity_map());
    auto singular_block = Block();
    auto const remaining_small_items_quantity = QuantityMap{
        {&item_0, 1},
        {&item_1, 1},
    };
    SECTION("case 1 - block made of item_1") {
        singular_block.add(
            item_1,
            DiscreteVector::zeros(),
            OrientationType::XYZ);
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 10);
    }
    SECTION("case 2 - block made of item_0") {
        singular_block.add(
            item_0,
            DiscreteVector::zeros(),
            OrientationType::XYZ);
        auto const waste = volume_loss.compute_measurement_waste(
            remaining_small_items_quantity,
            singular_block,
            input.large_object().measurement().x(),
            Axis::X);
        CHECK(waste == 4);
    }
}

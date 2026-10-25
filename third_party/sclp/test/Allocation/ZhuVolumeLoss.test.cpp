#include <catch.hpp>

#include <filesystem>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types/Measure.hpp"

auto const data_dir = std::filesystem::path("test/data/ZhuVolumeLoss/");

using namespace sclp;

// ---------- get_small_item_measurement ----------

TEST_CASE("get_small_item_measurement - set 0 - instance 1 (0)", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "00.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            {Axis::X, {{Axis::X, 108}, {Axis::Y, 108}, {Axis::Z, 0}}},
            {Axis::Y, {{Axis::X, 76}, {Axis::Y, 76}, {Axis::Z, 0}}},
            {Axis::Z, {{Axis::X, 0}, {Axis::Y, 0}, {Axis::Z, 30}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ZhuVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

TEST_CASE("get_small_item_measurement - set 0 - instance 2 (1)", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "01.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            {Axis::X, {{Axis::X, 49}, {Axis::Y, 49}, {Axis::Z, 0}}},
            {Axis::Y, {{Axis::X, 25}, {Axis::Y, 25}, {Axis::Z, 25}}},
            {Axis::Z, {{Axis::X, 21}, {Axis::Y, 21}, {Axis::Z, 21}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ZhuVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

TEST_CASE("get_small_item_measurement - set 0 - instance 6 (5)", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "02.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto const expected = std::map<Axis, std::map<Axis, CoordinateType>>{
        {
            {Axis::X, {{Axis::X, 86}, {Axis::Y, 86}, {Axis::Z, 86}}},
            {Axis::Y, {{Axis::X, 84}, {Axis::Y, 84}, {Axis::Z, 84}}},
            {Axis::Z, {{Axis::X, 45}, {Axis::Y, 45}, {Axis::Z, 45}}},
        },
    };
    auto actual = std::map<Axis, std::map<Axis, CoordinateType>>{};
    for (auto const & measurement_to_get : ALL_AXES) {
        for (auto const & axis : ALL_AXES) {
            actual[measurement_to_get][axis] = ZhuVolumeLoss::get_small_item_measurement(item, measurement_to_get, axis).value_or(0);
        }
    }
    CHECK(actual == expected);
}

// ---------- compute volume loss - operator() ----------

TEST_CASE("one item with one orientation", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "10.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    SECTION("one item remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (34 * 26 * 38)));
    }
    SECTION("two items remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 2},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (51 * 39 * 57)));
    }
}

#ifdef SCLP_DEBUG
TEST_CASE("update with a too large maximal measurement", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "10.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    CHECK_THROWS(volume_loss.update(input.quantity_map(), {1000, 1000, 1000}));
}
#endif

TEST_CASE("one item with some orientations", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "20.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    SECTION("one item remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (36 * 26 * 38)));
    }
    SECTION("two items remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 2},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (55 * 39 * 57)));
    }
}

TEST_CASE("one item with all orientations", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "30.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    SECTION("one item remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (36 * 32 * 38)));
    }
    SECTION("two items remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&item, 2},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (55 * 51 * 57)));
    }
}

TEST_CASE("two items, each with one orientations", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "40.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & items = input.small_items();
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(items[0], DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    SECTION("one item[0] remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&items[0], 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (34 * 34 * 34)));
    }
    SECTION("one item[1] remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&items[1], 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (40 * 40 * 40)));
    }
    SECTION("one of each remaining") {
        auto const remaining_small_items_quantity = QuantityMap{
            {&items[0], 1},
            {&items[1], 1},
        };
        volume_loss.update(remaining_small_items_quantity, empty_space.measurement());
        auto const waste = volume_loss(block, empty_space);
        CHECK(waste == ((75 * 77 * 80) - (57 * 57 * 57)));
    }
}

#ifdef SCLP_DEBUG
TEST_CASE("maximum measurement for update is larger than initial maximum measurement", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "50.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    auto const remaining_small_items_quantity = QuantityMap{
        {&item, 1},
    };
    CHECK_THROWS(volume_loss.update(remaining_small_items_quantity, empty_space.measurement()));
}
#endif

#ifdef SCLP_DEBUG
TEST_CASE("remaining space larger than container", "[ZhuVolumeLoss]") {
    auto const file_path = data_dir / "50.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const & item = input.small_items()[0];
    auto volume_loss = ZhuVolumeLoss(input.large_object());
    auto block = Block();
    block.add(item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto const empty_space = EmptySpace({75, 77, 80}, {1, 4, 9});
    auto const remaining_small_items_quantity = QuantityMap{
        {&item, 1},
    };
    volume_loss.update(remaining_small_items_quantity, input.large_object().measurement());
    CHECK_THROWS(volume_loss(block, empty_space));
}
#endif

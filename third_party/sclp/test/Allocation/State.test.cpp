#include <catch.hpp>

#include <filesystem>
#include <memory>

#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"

using namespace sclp;

auto const data_dir = std::filesystem::path("test/data/State/");

TEST_CASE("one item with one orientation enabled", "[State]") {
    auto const file_path = data_dir / "1.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const block_generator = GuillotineBlockGenerator(input.large_object(), 1.00, 1000);
    auto const blocks = block_generator.generate(input.small_items());
    auto state = State(input, blocks);
    SECTION("construction") {
        CHECK(state.valid());
        CHECK(state.packing.allocated_small_items().empty());
        CHECK(state.empty_spaces.size() == 1);
        CHECK(state.empty_spaces.top().first == EmptySpace(input.large_object().measurement(), DiscreteVector::zeros()));
        CHECK(state.blocks.size() == 1);
        CHECK(state.remaining_small_items_quantity == QuantityMap{{&input.small_items()[0], 1}});
        CHECK(state.hash == Hash());
    }
    SECTION("add the only block") {
        CHECK(state.valid());
        REQUIRE(state.blocks.size() == 1);
        state.add(state.blocks.front(), DiscreteVector{1, 2, 3});
        CHECK(state.empty_spaces.size() == 6);
        CHECK(state.blocks.size() == 0);
        CHECK(state.remaining_small_items_quantity == QuantityMap{});
        CHECK(state.hash != Hash());
    }
}

TEST_CASE("two items, each one with all orientations enabled", "[State]") {
    auto const file_path = data_dir / "2.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const block_generator = GuillotineBlockGenerator(input.large_object(), 1.00, 3);
    auto const blocks = block_generator.generate(input.small_items());
    auto state = State(input, blocks);
    SECTION("construction") {
        CHECK(state.valid());
        CHECK(state.packing.allocated_small_items().empty());
        CHECK(state.empty_spaces.size() == 1);
        CHECK(state.empty_spaces.top().first == EmptySpace(input.large_object().measurement(), DiscreteVector::zeros()));
        CHECK(state.blocks.size() == 3);
        auto const expected_quantity_map = QuantityMap{
            {&input.small_items()[0], 3},
            {&input.small_items()[1], 2},
        };
        CHECK(state.remaining_small_items_quantity == expected_quantity_map);
    }
    SECTION("add the only block") {
        auto const block = std::make_shared<Block>();
        block->add(input.small_items()[0], DiscreteVector::zeros(), OrientationType::XYZ);
        block->add(input.small_items()[1], {input.small_items()[0].measurement().x(), 0, 0}, OrientationType::XYZ);
        state.add(block, DiscreteVector::zeros());
        REQUIRE(state.blocks.size() == 0);
        CHECK_FALSE(state.valid());
        CHECK(state.empty_spaces.size() == 0);
        CHECK(state.blocks.size() == 0);
        auto const expected_quantity_map = QuantityMap{
            {&input.small_items()[0], 2},
            {&input.small_items()[1], 1},
        };
        CHECK(state.remaining_small_items_quantity == expected_quantity_map);
    }
}

TEST_CASE("_has_valid_space", "[State]") {
    auto const small_item = SmallItem(0, 1, {10, 20, 30});
    auto const block = std::make_shared<Block>();
    block->add(small_item, DiscreteVector::zeros(), OrientationType::XYZ);
    auto maximum_measurements = DiscreteVector{100, 200, 300};
    auto maximum_volume = Volume(100000);
    SECTION("valid block") {
        CHECK(State::_has_valid_space(block, maximum_measurements, maximum_volume));
    }
    SECTION("invalid measurement ") {
        SECTION("x") {
            maximum_measurements.x() = 9;
            CHECK_FALSE(State::_has_valid_space(block, maximum_measurements, maximum_volume));
        }
        SECTION("y") {
            maximum_measurements.y() = 9;
            CHECK_FALSE(State::_has_valid_space(block, maximum_measurements, maximum_volume));
        }
        SECTION("z") {
            maximum_measurements.z() = 9;
            CHECK_FALSE(State::_has_valid_space(block, maximum_measurements, maximum_volume));
        }
    }
    SECTION("invalid volume") {
        maximum_volume = Volume(9);
        CHECK_FALSE(State::_has_valid_space(block, maximum_measurements, maximum_volume));
    }
}

auto add_item(Block & block, SmallItem const & small_item, Index count = 1, Axis const & axis = Axis::Z) -> void {
    for (auto i = Index(0); i < count; ++i) {
        auto const position = DiscreteVector{
            axis == Axis::X ? block.measurement().x() : 0,
            axis == Axis::Y ? block.measurement().y() : 0,
            axis == Axis::Z ? block.measurement().z() : 0,
        };
        block.add(small_item, position, OrientationType::ALWAYS_ENABLED);
    }
    return;
}

TEST_CASE("_has_valid_quantities", "[State]") {
    auto const file_path = data_dir / "3.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const block_generator = GuillotineBlockGenerator(input.large_object(), 1.00, 1000);
    auto const blocks = block_generator.generate(input.small_items());
    auto state = State(input, blocks);
    auto block = Block();
    add_item(block, input.small_items()[0]);
    add_item(block, input.small_items()[1]);
    SECTION("valid quantities") {
        CHECK(state._has_valid_quantities(block));
    }
    SECTION("invalid quantity of first item") {
        add_item(block, input.small_items()[0], 4);
        CHECK_FALSE(state._has_valid_quantities(block));
    }
    SECTION("invalid quantity of second item") {
        add_item(block, input.small_items()[1], 4);
        CHECK_FALSE(state._has_valid_quantities(block));
    }
    SECTION("invalid quantity of unknown item") {
        auto const unknown_small_item = SmallItem(2, 1, {1, 2, 3});
        add_item(block, unknown_small_item, 1);
        CHECK_FALSE(state._has_valid_quantities(block));
    }
}

TEST_CASE("large case", "[State]") {
    auto const file_path = data_dir / "4.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const block_generator = GuillotineBlockGenerator(input.large_object(), 1.00, 1000);
    auto const blocks = block_generator.generate(input.small_items());
    auto state = State(input, blocks);
    SECTION("construction") {
        CHECK(state.valid());
        CHECK(state.packing.allocated_small_items().empty());
        CHECK(state.empty_spaces.size() == 1);
        CHECK(state.empty_spaces.top().first == EmptySpace(input.large_object().measurement(), DiscreteVector::zeros()));
        CHECK(state.blocks.size() > 1);
        auto expected_quantity_map = QuantityMap{};
        for (auto const & small_item : input.small_items()) {
            expected_quantity_map[&small_item] = small_item.quantity();
        }
        CHECK(state.remaining_small_items_quantity == expected_quantity_map);
    }
    SECTION("add two blocks") {
        auto blocks = std::vector{
            std::make_shared<Block>(),
            std::make_shared<Block>(),
        };
        add_item(*blocks[0], input.small_items()[0], 1);
        add_item(*blocks[0], input.small_items()[1], 1, Axis::X);
        add_item(*blocks[1], input.small_items()[1], 1);
        add_item(*blocks[1], input.small_items()[2], 1, Axis::Y);
        auto const expected_quantity_map = QuantityMap{
            {&input.small_items()[0], input.small_items()[0].quantity() - 1},
            {&input.small_items()[1], input.small_items()[1].quantity() - 2},
            {&input.small_items()[2], input.small_items()[2].quantity() - 1},
        };
        state.add(blocks[0], DiscreteVector::zeros()); // corner 0
        state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement()); // corner 8
        state.packing.build();
        CHECK(state.valid());
        CHECK(state.packing.allocated_small_items().size() == 4);
        CHECK(state.empty_spaces.size() == 6);
        CHECK(state.blocks.size() > 1);
        CHECK(state.remaining_small_items_quantity == expected_quantity_map);
    }
}

TEST_CASE("hash", "[State]") {
    auto const file_path = data_dir / "4.json";
    auto const data = FileLoader::json(file_path);
    auto const input = InputReader::read(data);
    auto const state = State(input, {});
    // create two blocks
    auto blocks = std::vector{
        std::make_shared<Block>(),
        std::make_shared<Block>(),
    };
    add_item(*blocks[0], input.small_items()[0], 1);
    add_item(*blocks[0], input.small_items()[1], 1, Axis::X);
    add_item(*blocks[1], input.small_items()[1], 1);
    add_item(*blocks[1], input.small_items()[2], 1, Axis::Y);
    auto const expected_quantity_map = QuantityMap{
        {&input.small_items()[0], input.small_items()[0].quantity() - 1},
        {&input.small_items()[1], input.small_items()[1].quantity() - 2},
        {&input.small_items()[2], input.small_items()[2].quantity() - 1},
    };
    // do not uncomment
    // this is here just so one knows better what pieces of code to write to add the blocks
    // state.add(blocks[0], DiscreteVector::zeros()); // corner 0
    // state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement()); // corner 8
    SECTION("a state with one block has different hash") {
        auto other_state = state;
        other_state.add(blocks[0], DiscreteVector::zeros());
        CHECK(state.hash != other_state.hash);
    }
    SECTION("two states with the same one block in the same position have the same hash") {
        auto first_state = state;
        first_state.add(blocks[0], DiscreteVector::zeros());
        auto second_state = state;
        second_state.add(blocks[0], DiscreteVector::zeros());
        CHECK(state.hash != first_state.hash);
        CHECK(state.hash != second_state.hash);
        CHECK(first_state.hash == second_state.hash);
    }
    SECTION("two states with the same two blocks in the same position added in the same order have the same hash") {
        auto first_state = state;
        first_state.add(blocks[0], DiscreteVector::zeros());
        first_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        auto second_state = state;
        second_state.add(blocks[0], DiscreteVector::zeros());
        second_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        CHECK(state.hash != first_state.hash);
        CHECK(state.hash != second_state.hash);
        CHECK(first_state.hash == second_state.hash);
    }
    SECTION("two states with the same two blocks in the same position added in the a different order have the same hash") {
        auto first_state = state;
        first_state.add(blocks[0], DiscreteVector::zeros());
        first_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        auto second_state = state;
        second_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        second_state.add(blocks[0], DiscreteVector::zeros());
        CHECK(state.hash != first_state.hash);
        CHECK(state.hash != second_state.hash);
        CHECK(first_state.hash == second_state.hash);
    }
    SECTION("two states with the same two blocks in different positions added in the same order have different hashes") {
        auto first_state = state;
        first_state.add(blocks[0], DiscreteVector{1, 0, 0});
        first_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        auto second_state = state;
        second_state.add(blocks[0], DiscreteVector::zeros());
        second_state.add(blocks[1], input.large_object().measurement() - blocks[1]->measurement());
        CHECK(state.hash != first_state.hash);
        CHECK(state.hash != second_state.hash);
        CHECK(first_state.hash != second_state.hash);
    }
}

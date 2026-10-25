#include <catch.hpp>

#include <filesystem>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Packing/Block.hpp"

auto const data_dir = std::filesystem::path("test/data/GuillotineBlockGenerator/");

using namespace sclp;

auto make_small_item(DiscreteVector measurement, Quantity quantity, Index index = 0) {
    return SmallItem(index, quantity, measurement, sclp::EnabledOrientations::all());
}

auto make_guillotine_block_generator(
    DiscreteVector const & measurement = DiscreteVector{100, 100, 100},
    double const & minimum_volume_ratio = 0.0,
    Index const & maximum_number_of_blocks = 100) -> GuillotineBlockGenerator {
    return GuillotineBlockGenerator(LargeObject(measurement), minimum_volume_ratio, maximum_number_of_blocks);
}

TEST_CASE("guillotine block join", "[GuillotineBlockGenerator]") {
    auto const small_items = std::vector{
        make_small_item(DiscreteVector{2, 5, 11}, 2, 0),
        make_small_item(DiscreteVector{3, 7, 13}, 2, 1),
    };
    auto const generator = make_guillotine_block_generator();
    auto const blocks = std::vector{
        std::make_shared<Block>(),
        std::make_shared<Block>(),
    };
    blocks[0]->add(small_items[0], DiscreteVector::zeros(), OrientationType::XYZ);
    blocks[0]->update_hash();
    blocks[1]->add(small_items[1], DiscreteVector::zeros(), OrientationType::XYZ);
    blocks[1]->update_hash();
    auto expected_quantity_map = QuantityMap{
        {&small_items[0], 1},
        {&small_items[1], 1},
    };
    SECTION("single item in a block") {
        auto const & small_item = small_items[0];
        auto block = Block();
        block.add(small_item, DiscreteVector::zeros(), OrientationType::XYZ);
        CHECK(block.allocated_small_items().size() == 1);
        CHECK(block.measurement() == small_item.measurement());
        CHECK(block.volume() == small_item.measurement().prod());
        CHECK(block.quantity_map() == QuantityMap{{&small_item, 1}});
        CHECK(block.valid_quantity());
        CHECK(block.occupied_volume_ratio() == Catch::Approx(1.0));
    }
    SECTION("oriented single item in a block") {
        auto const & small_item = small_items[0];
        auto block = Block();
        block.add(small_item, DiscreteVector::zeros(), OrientationType::ZYX);
        CHECK(block.allocated_small_items().size() == 1);
        CHECK(block.measurement() == DiscreteVector{11, 5, 2});
        CHECK(block.volume() == Volume(11 * 5 * 2));
        CHECK(block.quantity_map() == QuantityMap{{&small_item, 1}});
        CHECK(block.valid_quantity());
        CHECK(block.occupied_volume_ratio() == Catch::Approx(1.0));
    }
    SECTION("join x") {
        auto const block_join = generator.join(blocks[0], blocks[1], Axis::X);
        CHECK(block_join->allocated_small_items().size() == 2);
        CHECK(block_join->measurement() == DiscreteVector{5, 7, 13});
        CHECK(block_join->volume() == Volume(5 * 7 * 13));
        CHECK(block_join->quantity_map() == expected_quantity_map);
        CHECK(block_join->valid_quantity());
        CHECK(block_join->occupied_volume_ratio() == Catch::Approx(0.8417582417582418));
    }
    SECTION("join y") {
        auto const block_join = generator.join(blocks[0], blocks[1], Axis::Y);
        CHECK(block_join->allocated_small_items().size() == 2);
        CHECK(block_join->measurement() == DiscreteVector{3, 12, 13});
        CHECK(block_join->volume() == Volume(3 * 12 * 13));
        CHECK(block_join->quantity_map() == expected_quantity_map);
        CHECK(block_join->valid_quantity());
        CHECK(block_join->occupied_volume_ratio() == Catch::Approx(0.8183760683760684));
    }
    SECTION("join z") {
        auto const block_join = generator.join(blocks[0], blocks[1], Axis::Z);
        CHECK(block_join->allocated_small_items().size() == 2);
        CHECK(block_join->measurement() == DiscreteVector{3, 7, 24});
        CHECK(block_join->volume() == Volume(3 * 7 * 24));
        CHECK(block_join->quantity_map() == expected_quantity_map);
        CHECK(block_join->valid_quantity());
        CHECK(block_join->occupied_volume_ratio() == Catch::Approx(0.7599206349206349));
    }
    SECTION("join x, y, z") {
        auto blocks_join = std::vector<Block::Sptr>{};
        blocks_join.push_back(blocks[0]);
        blocks_join.push_back(blocks[1]);
        blocks_join.push_back(generator.join(blocks_join[0], blocks_join[1], Axis::X));
        blocks_join.push_back(generator.join(blocks_join[1], blocks_join[2], Axis::Y));
        blocks_join.push_back(generator.join(blocks_join[2], blocks_join[3], Axis::Z));
        expected_quantity_map[&small_items[0]] = 2;
        expected_quantity_map[&small_items[1]] = 3;
        CHECK(blocks_join.back()->allocated_small_items().size() == 5);
        CHECK(blocks_join.back()->measurement() == DiscreteVector{5, 14, 26});
        CHECK(blocks_join.back()->volume() == Volume(5 * 14 * 26));
        CHECK(blocks_join.back()->quantity_map() == expected_quantity_map);
        CHECK(blocks_join.back()->valid_quantity() == false);
        CHECK(blocks_join.back()->occupied_volume_ratio() == Catch::Approx(0.5708791209));
    }
}

TEST_CASE("guillotine block generate - limit by quantity", "[GuillotineBlockGenerator]") {
    auto const generator = make_guillotine_block_generator();
    SECTION("generate none") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 0, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 0, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 0);
    }
    SECTION("generate from one item") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 1);
    }
    SECTION("generate from one item with orientation") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{2, 3, 5}, 1),
        };
        REQUIRE(small_items.back().orientation().size() == 6);
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 6);
    }
    SECTION("generate from 2") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 2),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 4);
    }
    SECTION("generate some") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 1, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 5);
    }
    SECTION("generate many") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 2, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 17);
    }
}

TEST_CASE("guillotine block generate - limit by volume ratio", "[GuillotineBlockGenerator]") {
    auto const generator = make_guillotine_block_generator({100, 100, 100}, 1.0, 100);
    SECTION("generate from one of each small item") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 1, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 2);
    }
    SECTION("generate some") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 1, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 2);
    }
    SECTION("generate many") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 4, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 1, 1),
        };
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 17);
    }
}

TEST_CASE("guillotine block generate - limit by space", "[GuillotineBlockGenerator]") {
    SECTION("generate nothing") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 1, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 1, 1),
        };
        auto const generator = make_guillotine_block_generator({0, 0, 0});
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 0);
    }
    SECTION("generate one") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 100, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 100, 1),
        };
        auto const generator = make_guillotine_block_generator({1, 1, 1});
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 1);
    }
    SECTION("generate two") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 100, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 100, 1),
        };
        auto const generator = make_guillotine_block_generator({2, 1, 1});
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 2);
    }
    SECTION("generate many") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{1, 1, 1}, 100, 0),
            make_small_item(DiscreteVector{2, 2, 2}, 100, 1),
        };
        auto const generator = make_guillotine_block_generator({2, 2, 2});
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 16);
    }
}

TEST_CASE("guillotine block generate - orientation", "[GuillotineBlockGenerator]") {
    auto const minimum_volume_ratio = 1.0;
    auto const maximum_number_of_blocks = Index(100);
    SECTION("generate with one") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{2, 5, 13}, 1),
        };
        auto const generator = make_guillotine_block_generator({100, 100, 100}, minimum_volume_ratio, maximum_number_of_blocks);
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 6);
    }
    SECTION("generate with two") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{2, 5, 13}, 2),
        };
        auto const generator = make_guillotine_block_generator({100, 100, 100}, minimum_volume_ratio, maximum_number_of_blocks);
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 24);
    }
    SECTION("generate with two with limited space") {
        auto const small_items = std::vector{
            make_small_item(DiscreteVector{2, 5, 13}, 2),
        };
        auto const generator = make_guillotine_block_generator({5, 5, 13}, minimum_volume_ratio, maximum_number_of_blocks);
        auto const blocks = generator.generate(small_items);
        CHECK(blocks.size() == 4);
    }
}

TEST_CASE("guillotine block generate - limit by number of blocks", "[GuillotineBlockGenerator]") {
    auto const small_items = std::vector{
        make_small_item(DiscreteVector{2, 3, 5}, 100,0 ),
        make_small_item(DiscreteVector{7, 11, 13}, 100, 1),
        make_small_item(DiscreteVector{17, 19, 23}, 100, 2),
        make_small_item(DiscreteVector{29, 31, 37}, 100, 3),
        make_small_item(DiscreteVector{41, 43, 47}, 100, 4),
        make_small_item(DiscreteVector{53, 59, 61}, 100, 5),
        make_small_item(DiscreteVector{67, 71, 73}, 100, 6),
        make_small_item(DiscreteVector{79, 83, 89}, 100, 7),
    };
    auto const generator = make_guillotine_block_generator({500, 500, 500});
    auto const blocks = generator.generate(small_items);
    CHECK(blocks.size() == generator.maximum_number_of_blocks());
}

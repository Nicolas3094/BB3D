#include <catch.hpp>

#include <filesystem>
#include <string>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"

auto const data_dir = std::filesystem::path("test/data/GuillotineBlockGenerator");

using namespace sclp;

// check that the executions of this test does
// take longer than a predefined time

auto _run(std::string const & file_name, Index repetitions = 1, Index const expected_number_of_blocks = 0) -> void {
    REQUIRE(repetitions > 0);
    auto const file_path = data_dir / file_name;
    auto const data = sclp::FileLoader::json(file_path);
    auto const input = sclp::InputReader::read(data);
    auto const minimum_volume_ratio = double(0.98);
    auto const maximum_number_of_blocks = Index(10000);
    auto const generator = GuillotineBlockGenerator(input.large_object(), minimum_volume_ratio, maximum_number_of_blocks);
    auto blocks_generated = std::vector<std::vector<Block::Sptr>>();
    for (Index i = 0; i < repetitions; ++i) {
        blocks_generated.push_back(generator.generate(input.small_items()));
    }
    // check that blocks were produced
    CHECK(!blocks_generated.back().empty());
    // check for deterministic behavior
    for (auto const & blocks : blocks_generated) {
        CHECK(blocks.size() == expected_number_of_blocks);
    }
    return;
}

TEST_CASE("set_12_instance_1", "[GuillotineBlockGenerator]") {
    _run("set_12_instance_1_reduced.json", 2, 1674);
}

TEST_CASE("set_1_instance_3", "[GuillotineBlockGenerator]") {
    _run("set_1_instance_3.json", 3, 1458);
}

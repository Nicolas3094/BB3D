#include <catch.hpp>

#include <filesystem>

#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"

auto const data_dir = std::filesystem::path("test/data/IO/");

using namespace sclp;

TEST_CASE("constructor", "[InputReader]") {
    const auto file_path = data_dir / "InputReader.json";
    const auto data = FileLoader::json(file_path);
    const auto input = InputReader::read(data);
    SECTION("large_object") {
        CHECK(input.large_object().measurement() == DiscreteVector{1000, 2000, 3000});
    }
    SECTION("small_items") {
        CHECK(input.small_items().size() == 4u);
        CHECK(input.small_items()[0].quantity() == 41);
        CHECK(input.small_items()[0].measurement() == DiscreteVector{3, 5, 7});
        CHECK(input.small_items()[1].quantity() == 97);
        CHECK(input.small_items()[1].measurement() == DiscreteVector{11, 13, 19});
    }
    SECTION("aggregate properties") {
        CHECK(input.total_quantity() == 193);
        CHECK(input.total_volume() == 1494197);
    }
}

#include <catch.hpp>

#include <filesystem>

#include "sclp/Constraint/NonEmptyPackingConstraint.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Input.hpp"

using namespace sclp;

auto const data_dir = std::filesystem::path("test/data/Constraint/");

TEST_CASE("0", "[Constraint][NonEmptyPackingConstraint]") {
    const auto data = sclp::FileLoader::json(data_dir / "00.json"); // empty input
    const auto input = sclp::InputReader::read(data);
    auto packing = Packing{};
    const auto result = NonEmptyPackingConstraint::check(input, packing);
    CHECK(result == true);
}

TEST_CASE("01", "[Constraint][NonEmptyPackingConstraint]") {
    const auto data = sclp::FileLoader::json(data_dir / "01.json");
    const auto input = sclp::InputReader::read(data);
    const auto & i0 = input.small_items()[0];
    auto packing = Packing{};
    SECTION("empty packing") {
        const auto result = NonEmptyPackingConstraint::check(input, packing);
        CHECK(result == false);
    }
    SECTION("valid") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        const auto result = NonEmptyPackingConstraint::check(input, packing);
        CHECK(result == true);
    }
}

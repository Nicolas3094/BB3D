#include <catch.hpp>

#include <filesystem>

#include "sclp/Constraint/AllConstraints.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Input.hpp"

using namespace sclp;

auto const data_dir = std::filesystem::path("test/data/Constraint/");

TEST_CASE("0", "[Constraint][NonEmptyPackingConstraint]") {
    const auto data = sclp::FileLoader::json(data_dir / "00.json"); // empty input
    const auto input = sclp::InputReader::read(data);
    auto packing = Packing{};
    const auto result = AllConstraints::check(input, packing);
    CHECK(result == true);
}

TEST_CASE("01", "[Constraint][AllConstraints]") {
    const auto data = sclp::FileLoader::json(data_dir / "01.json");
    const auto input = sclp::InputReader::read(data);
    const auto & i0 = input.small_items()[0];
    const auto & i1 = input.small_items()[1];
    auto packing = Packing{};
    SECTION("all constraints met") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        packing.add(i0, DiscreteVector{0, 0, 7}, OrientationType::YXZ);
        packing.add(i1, DiscreteVector{5, 0, 0}, OrientationType::XYZ);
        packing.add(i1, DiscreteVector{5, 0, 19}, OrientationType::ZYX);
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == true);
    }
    SECTION("non empty packing violated") {
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == false);
    }
    SECTION("overlap violation") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        packing.add(i0, DiscreteVector{0, 0, 7}, OrientationType::YXZ);
        packing.add(i1, DiscreteVector{4, 0, 0}, OrientationType::XYZ);
        packing.add(i1, DiscreteVector{5, 0, 19}, OrientationType::ZYX);
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == false);
    }
    SECTION("large object space violated") {
        packing.add(i0, DiscreteVector{1000, 0, 0}, OrientationType::XYZ);
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == false);
    }
    SECTION("quantity violated") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        packing.add(i0, DiscreteVector{0, 0, 7}, OrientationType::YXZ);
        packing.add(i0, DiscreteVector{0, 0, 14}, OrientationType::YXZ);
        packing.add(i1, DiscreteVector{5, 0, 0}, OrientationType::XYZ);
        packing.add(i1, DiscreteVector{5, 0, 19}, OrientationType::ZYX);
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == false);
    }
    SECTION("orientation violated") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::YZX);
        const auto result = AllConstraints::check(input, packing);
        CHECK(result == false);
    }
}

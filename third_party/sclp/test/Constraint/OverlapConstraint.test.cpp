#include <catch.hpp>

#include <filesystem>

#include "sclp/Constraint/OverlapConstraint.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Input.hpp"

using namespace sclp;

auto const data_dir = std::filesystem::path("test/data/Constraint/");

TEST_CASE("01", "[Constraint][OverlapConstraint]") {
    const auto data = sclp::FileLoader::json(data_dir / "01.json");
    const auto input = sclp::InputReader::read(data);
    const auto & i0 = input.small_items()[0];
    const auto & i1 = input.small_items()[1];
    auto packing = Packing{};
    SECTION("empty packing") {
        const auto result = OverlapConstraint::check(packing);
        CHECK(result.valid == true);
        CHECK(result.overlaps.empty());
    }
    SECTION("all constraints met") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        packing.add(i0, DiscreteVector{0, 0, 7}, OrientationType::YXZ);
        packing.add(i1, DiscreteVector{5, 0, 0}, OrientationType::XYZ);
        packing.add(i1, DiscreteVector{5, 0, 19}, OrientationType::ZYX);
        const auto result = OverlapConstraint::check(packing);
        CHECK(result.valid == true);
        CHECK(result.overlaps.empty());
    }
    SECTION("overlap violation") {
        packing.add(i0, DiscreteVector{0, 0, 0}, OrientationType::XYZ);
        packing.add(i0, DiscreteVector{0, 0, 7}, OrientationType::YXZ);
        packing.add(i1, DiscreteVector{4, 0, 0}, OrientationType::XYZ);
        packing.add(i1, DiscreteVector{5, 0, 19}, OrientationType::ZYX);
        const auto expected_overlaps = OverlapConstraint::OverlapPairsList{
            {&packing.allocated_small_items()[1], &packing.allocated_small_items()[2]},
        };
        const auto result = OverlapConstraint::check(packing);
        CHECK(result.valid == false);
        CHECK(result.overlaps == expected_overlaps);
    }
}

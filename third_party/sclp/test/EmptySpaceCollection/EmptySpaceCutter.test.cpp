#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/EmptySpaceCollection/EmptySpaceCutter.hpp"
#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Packing/EmptySpace.hpp"

using namespace sclp;

auto check_unordered_equals(const std::vector<EmptySpace> & lhs, const std::vector<EmptySpace> & rhs) -> bool {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    const auto size = lhs.size();
    for (auto i = 0u; i < size; ++i) {
        bool equal_element_found = false;
        for (auto j = 0u; j < size; ++j) {
            if (lhs[i] == rhs[j]) {
                equal_element_found = true;
                break;
            }
        }
        if (!equal_element_found) {
            return false;
        }
    }
    return true;
}

TEST_CASE("EmptySpaceCutter", "[EmptySpaceCutter]") {
    const auto empty_space_position = DiscreteVector{10, 20, 30};
    const auto empty_space_measurement = DiscreteVector{20, 30, 40};
    auto empty_space = EmptySpace(empty_space_measurement, empty_space_position);
    SECTION("case 0 - the AllocatedSmallItem and the EmptySpace have no overlap") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{200, 300, 400}, DiscreteVector{100, 200, 300});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 1 - the AllocatedSmallItem intersects one corner of the EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{5, 7, 11}, DiscreteVector{9, 19, 29});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector{16, 30, 40}, DiscreteVector{14, 20, 30});
        expected.emplace_back(DiscreteVector{20, 24, 40}, DiscreteVector{10, 26, 30});
        expected.emplace_back(DiscreteVector{20, 30, 30}, DiscreteVector{10, 20, 40});
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 2 - the AllocatedSmallItem intersects one entire edge of the EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{10, 30, 5}, DiscreteVector{10, 20, 65});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector{20, 30, 35}, DiscreteVector{10, 20, 30});
        expected.emplace_back(DiscreteVector{10, 30, 40}, DiscreteVector{20, 20, 30});
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 3 - the AllocatedSmallItem intersects part of one edge of the EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{25, 10, 5}, DiscreteVector{20, 29, 30});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector{20, 30, 35}, DiscreteVector{10, 20, 35});
        expected.emplace_back(DiscreteVector{10, 30, 40}, DiscreteVector{10, 20, 30});
        expected.emplace_back(DiscreteVector{20, 11, 40}, DiscreteVector{10, 39, 30});
        expected.emplace_back(DiscreteVector{20, 9, 40}, DiscreteVector{10, 20, 30});
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 5 - the AllocatedSmallItem cuts away an entire plane of the EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{5, 30, 40}, DiscreteVector{10, 20, 30});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector{15, 30, 40}, DiscreteVector{15, 20, 30});
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 9 - the whole AllocatedSmallItem resides in the EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{5, 7, 10}, DiscreteVector{20, 35, 50});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector{10, 30, 40}, DiscreteVector{10, 20, 30});
        expected.emplace_back(DiscreteVector{20, 30, 20}, DiscreteVector{10, 20, 30});
        expected.emplace_back(DiscreteVector{20, 15, 40}, DiscreteVector{10, 20, 30});
        expected.emplace_back(DiscreteVector{5, 30, 40}, DiscreteVector{25, 20, 30});
        expected.emplace_back(DiscreteVector{20, 8, 40}, DiscreteVector{10, 42, 30});
        expected.emplace_back(DiscreteVector{20, 30, 10}, DiscreteVector{10, 20, 60});
        CHECK(check_unordered_equals(actual, expected));
    }
    SECTION("case 10 - the AllocatedSmallItem covers the whole EmptySpace") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(DiscreteVector{100, 100, 100}, DiscreteVector{0, 0, 0});
        auto actual = std::vector<EmptySpace>{};
        EmptySpaceCutter::cut(empty_space, allocated_rectangular_cuboid, actual);
        auto expected = std::vector<EmptySpace>{};
        CHECK(check_unordered_equals(actual, expected));
    }
    // TODO: cases not tested:
    // "case 4 - the AllocatedSmallItem gets into the EmptySpace from one plane without touching any corner point"
    // "case 6 - the AllocatedSmallItem crosses one pair of parallel edges of the EmptySpace without touching any corner of the EmptySpace"
    // "case 7 - the AllocatedSmallItem passes through one pair of parallel planes of the EmptySpace without any corner point of the EmptySpace"
    // "case 8 - the AllocatedSmallItem cuts the EmptySpace into two new empty spaces"
}

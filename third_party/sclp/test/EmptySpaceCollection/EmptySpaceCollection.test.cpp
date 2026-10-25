#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/EmptySpaceCollection/EmptySpaceCollection.hpp"
#include "sclp/Packing/EmptySpace.hpp"

using namespace sclp;

template <typename EmptySpaceCollectionType>
auto get_collection_as_vector(EmptySpaceCollectionType & collection) {
    auto result = std::vector<EmptySpace>{};
    while (!collection.empty()) {
        const auto & [empty_space, _] = collection.top();
        result.emplace_back(empty_space.measurement(), empty_space.initial_position()); // this is a copy
        collection.pop();
    }
    return result;
}

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

using EmptySpaceCollection8 = EmptySpaceCollection;

TEST_CASE("ordering by anchor distance - Anchor8", "[EmptySpaceCollection][Anchor8]") {
    auto const large_space_measurements = DiscreteVector{{100, 100, 100}};
    auto empty_space_collection = EmptySpaceCollection8(large_space_measurements);
    empty_space_collection.add(EmptySpace{DiscreteVector(7, 7, 7), DiscreteVector(70, 70, 70)});
    empty_space_collection.add(EmptySpace{DiscreteVector(9, 9, 9), DiscreteVector(0, 0, 0)});
    empty_space_collection.add(EmptySpace{DiscreteVector(6, 6, 6), DiscreteVector(40, 40, 40)});
    empty_space_collection.add(EmptySpace{DiscreteVector(8, 8, 8), DiscreteVector(90, 90, 90)});
    CHECK(empty_space_collection.maximum_measurement() == DiscreteVector{9, 9, 9});
    CHECK(empty_space_collection.maximum_volume() == 729);
    const auto actual = get_collection_as_vector(empty_space_collection);
    // sort them by anchor distance, the lower the distance, the higher (first in the vector) it is
    auto expected = std::vector<EmptySpace>{};
    expected.emplace_back(EmptySpace{DiscreteVector(9, 9, 9), DiscreteVector(0, 0, 0)});
    expected.emplace_back(EmptySpace{DiscreteVector(8, 8, 8), DiscreteVector(90, 90, 90)});
    expected.emplace_back(EmptySpace{DiscreteVector(7, 7, 7), DiscreteVector(70, 70, 70)});
    expected.emplace_back(EmptySpace{DiscreteVector(6, 6, 6), DiscreteVector(40, 40, 40)});
    CHECK(actual.size() == expected.size());
    for (auto i = 0u; i < expected.size(); ++i) {
        CHECK((actual[i] == expected[i]));
    }
}

TEST_CASE("ordering by volume - Anchor8", "[EmptySpaceCollection][Anchor8]") {
    auto const large_space_measurements = DiscreteVector{{100, 100, 100}};
    auto empty_space_collection = EmptySpaceCollection8(large_space_measurements);
    empty_space_collection.add(EmptySpace{DiscreteVector(1, 1, 1), DiscreteVector(1, 1, 1)});
    empty_space_collection.add(EmptySpace{DiscreteVector(9, 9, 9), DiscreteVector(90, 90, 90)});
    empty_space_collection.add(EmptySpace{DiscreteVector(8, 8, 8), DiscreteVector(91, 1, 1)});
    empty_space_collection.add(EmptySpace{DiscreteVector(5, 5, 5), DiscreteVector(1, 94, 1)});
    const auto actual = get_collection_as_vector(empty_space_collection);
    // they all have the same anchor distance: {1, 1, 1}, but they have different volumes
    auto expected = std::vector<EmptySpace>{};
    expected.emplace_back(EmptySpace{DiscreteVector(9, 9, 9), DiscreteVector(90, 90, 90)});
    expected.emplace_back(EmptySpace{DiscreteVector(8, 8, 8), DiscreteVector(91, 1, 1)});
    expected.emplace_back(EmptySpace{DiscreteVector(5, 5, 5), DiscreteVector(1, 94, 1)});
    expected.emplace_back(EmptySpace{DiscreteVector(1, 1, 1), DiscreteVector(1, 1, 1)});
    CHECK(actual.size() == expected.size());
    for (auto i = 0u; i < expected.size(); ++i) {
        CHECK((actual[i] == expected[i]));
    }
}

TEST_CASE("cut - Anchor8", "[EmptySpaceCollection][Anchor8]") {
    auto const large_space_measurements = DiscreteVector{{100, 100, 100}};
    auto empty_space_collection = EmptySpaceCollection8(large_space_measurements);
    CHECK(empty_space_collection.empty());
    empty_space_collection.add({DiscreteVector(9, 9, 9), DiscreteVector(9, 9, 9)});
    empty_space_collection.add({DiscreteVector(1, 1, 1), DiscreteVector(0, 0, 0)});
    empty_space_collection.add({DiscreteVector(1, 1, 1), DiscreteVector(1, 0, 0)});
    empty_space_collection.add({DiscreteVector(1, 1, 1), DiscreteVector(0, 1, 0)});
    empty_space_collection.add({DiscreteVector(1, 1, 1), DiscreteVector(0, 1, 0)});
    SECTION("cut") {
        const auto allocated_rectangular_cuboid = sclp::AllocatedRectangularCuboid(
            DiscreteVector(1, 1, 1),
            DiscreteVector(1, 0, 0));
        empty_space_collection.cut(allocated_rectangular_cuboid);
        const auto actual = get_collection_as_vector(empty_space_collection);
        auto expected = std::vector<EmptySpace>{};
        expected.emplace_back(DiscreteVector(9, 9, 9), DiscreteVector(9, 9, 9));
        expected.emplace_back(DiscreteVector(1, 1, 1), DiscreteVector(0, 0, 0));
        expected.emplace_back(DiscreteVector(1, 1, 1), DiscreteVector(0, 1, 0));
        CHECK(check_unordered_equals(actual, expected));
    }
}

#include <catch.hpp>

#include <vector>

#include "sclp/Geometry/AllocatedObject.hpp"

using namespace sclp;

TEST_CASE("interface", "[AllocatedObject][Geometry]") {
    auto const initial_position = DiscreteVector{3, 5, 7};
    auto const final_position = DiscreteVector{11, 13, 17};
    auto const allocated_object = AllocatedObject(initial_position, final_position);
    CHECK(allocated_object.initial_position() == initial_position);
    CHECK(allocated_object.final_position() == final_position);
}

TEST_CASE("equality", "[AllocatedObject][Geometry]") {
    auto const original = AllocatedObject({1, 2, 3}, {10, 20, 30});
    auto others = std::vector<AllocatedObject>();
    others.emplace_back(DiscreteVector{1, 2, 3}, DiscreteVector{10, 20, 30});
    others.emplace_back(DiscreteVector{9, 2, 3}, DiscreteVector{10, 20, 30});
    others.emplace_back(DiscreteVector{1, 9, 3}, DiscreteVector{10, 20, 30});
    others.emplace_back(DiscreteVector{1, 2, 9}, DiscreteVector{10, 20, 30});
    others.emplace_back(DiscreteVector{1, 2, 3}, DiscreteVector{99, 20, 30});
    others.emplace_back(DiscreteVector{1, 2, 3}, DiscreteVector{10, 99, 30});
    others.emplace_back(DiscreteVector{1, 2, 3}, DiscreteVector{10, 20, 99});
    SECTION("operator==") {
        SECTION("true") {
            CHECK(original == original);
            CHECK(original == others[0]);
        }
        SECTION("false") {
            for (auto i = 0u; i < others.size(); ++i) {
                for (auto j = i + 1; j < others.size(); ++j) {
                    CHECK_FALSE(others[i] == others[j]);
                }
            }
        }
    }
}

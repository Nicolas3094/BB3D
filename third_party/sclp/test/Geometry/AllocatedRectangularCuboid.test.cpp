#include <catch.hpp>

#include <vector>

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

TEST_CASE("constructor", "[AllocatedRectangularCuboid][Geometry]") {
    auto const measurement = DiscreteVector{1, 2, 3};
    auto const initial_position = DiscreteVector{5, 7, 11};
    auto const final_position = DiscreteVector{6, 9, 14};
    auto const cuboid = AllocatedRectangularCuboid(measurement, initial_position);
    CHECK(cuboid.measurement() == measurement);
    CHECK(cuboid.initial_position() == initial_position);
    CHECK(cuboid.final_position() == final_position);
}

TEST_CASE("overlaps", "[AllocatedRectangularCuboid][Geometry]") {
    auto const cuboid = AllocatedRectangularCuboid({3, 5, 7}, {30, 50, 70});
    SECTION("true") {
        auto others = std::vector<AllocatedRectangularCuboid>();
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 50, 70});
        others.emplace_back(DiscreteVector{1, 3, 7}, DiscreteVector{31, 51, 71});
        others.emplace_back(DiscreteVector{5, 7, 9}, DiscreteVector{29, 49, 69});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{29, 50, 70});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 49, 70});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 50, 69});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{29, 49, 69});
        for (auto const & other : others) {
            CHECK(cuboid.overlaps(other));
            CHECK(other.overlaps(cuboid));
            CHECK(cuboid.overlaps_or_touches(other));
            CHECK(other.overlaps_or_touches(cuboid));
            CHECK_FALSE(cuboid.touches(other));
            CHECK_FALSE(other.touches(cuboid));
        }
    }
    SECTION("false") {
        auto others = std::vector<AllocatedRectangularCuboid>();
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 50, 70});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 55, 70});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 50, 77});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 55, 77});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{300, 500, 700});
        others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{0, 0, 0});
        for (auto const & other : others) {
            CHECK_FALSE(cuboid.overlaps(other));
            CHECK_FALSE(other.overlaps(cuboid));
        }
    }
}

TEST_CASE("touches", "[AllocatedRectangularCuboid][Geometry]") {
    auto const cuboid = AllocatedRectangularCuboid({3, 5, 7}, {30, 50, 70});
    SECTION("true") {
        SECTION("one point of contact") {
            auto others = std::vector<AllocatedRectangularCuboid>();
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 45, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 45, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 55, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 55, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 45, 77});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 45, 77});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 55, 77});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 55, 77});
            for (auto const & other : others) {
                CHECK(cuboid.touches(other));
                CHECK(other.touches(cuboid));
                CHECK(cuboid.overlaps_or_touches(other));
                CHECK(other.overlaps_or_touches(cuboid));
                CHECK_FALSE(cuboid.overlaps(other));
                CHECK_FALSE(other.overlaps(cuboid));
            }
        }
        SECTION("one line of contact") {
            auto others = std::vector<AllocatedRectangularCuboid>();
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{28, 45, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 48, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{29, 55, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 52, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 45, 74});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 45, 77});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 55, 73});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{31, 55, 77});
            for (auto const & other : others) {
                CHECK(cuboid.touches(other));
                CHECK(other.touches(cuboid));
                CHECK(cuboid.overlaps_or_touches(other));
                CHECK(other.overlaps_or_touches(cuboid));
                CHECK_FALSE(cuboid.overlaps(other));
                CHECK_FALSE(other.overlaps(cuboid));
            }
        }
        SECTION("one face of contact") {
            auto others = std::vector<AllocatedRectangularCuboid>();
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{28, 49, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 48, 67});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{28, 53, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{33, 52, 66});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{28, 45, 74});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{31, 49, 77});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{27, 52, 73});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{31, 53, 77});
            for (auto const & other : others) {
                CHECK(cuboid.touches(other));
                CHECK(other.touches(cuboid));
                CHECK(cuboid.overlaps_or_touches(other));
                CHECK(other.overlaps_or_touches(cuboid));
                CHECK_FALSE(cuboid.overlaps(other));
                CHECK_FALSE(other.overlaps(cuboid));
            }
        }
    }
    SECTION("false") {
        SECTION("no overlap") {
            auto others = std::vector<AllocatedRectangularCuboid>();
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{26, 45, 63});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{36, 55, 63});
            for (auto const & other : others) {
                CHECK_FALSE(cuboid.touches(other));
                CHECK_FALSE(other.touches(cuboid));
                CHECK_FALSE(cuboid.overlaps(other));
                CHECK_FALSE(other.overlaps(cuboid));
                CHECK_FALSE(cuboid.overlaps_or_touches(other));
                CHECK_FALSE(other.overlaps_or_touches(cuboid));
            }
        }
        SECTION("overlaps") {
            auto others = std::vector<AllocatedRectangularCuboid>();
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 50, 70});
            others.emplace_back(DiscreteVector{1, 3, 7}, DiscreteVector{31, 51, 71});
            others.emplace_back(DiscreteVector{5, 7, 9}, DiscreteVector{29, 49, 69});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{29, 50, 70});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 49, 70});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{30, 50, 69});
            others.emplace_back(DiscreteVector{3, 5, 7}, DiscreteVector{29, 49, 69});
            for (auto const & other : others) {
                CHECK_FALSE(cuboid.touches(other));
                CHECK_FALSE(other.touches(cuboid));
                CHECK(cuboid.overlaps_or_touches(other));
                CHECK(other.overlaps_or_touches(cuboid));
                CHECK(cuboid.overlaps(other));
                CHECK(other.overlaps(cuboid));
            }
        }
    }
}

TEST_CASE("contains", "[AllocatedRectangularCuboid][Geometry]") {
    auto const big = AllocatedRectangularCuboid({10, 10, 10}, {0, 0, 0});
    auto const small = AllocatedRectangularCuboid({1, 1, 1}, {0, 0, 0});
    auto const small_with_offset = AllocatedRectangularCuboid({1, 1, 1}, {4, 4, 4});
    auto const outside = AllocatedRectangularCuboid({2, 2, 2}, {0, 0, 9});
    CHECK(big.contains(big));
    CHECK(big.contains(small));
    CHECK(big.contains(small_with_offset));
    CHECK_FALSE(small.contains(outside));
    CHECK_FALSE(big.contains(outside));
}

TEST_CASE("equality", "[AllocatedRectangularCuboid][Geometry]") {
    auto const original = AllocatedRectangularCuboid({1, 2, 3}, {10, 20, 30});
    auto others = std::vector<AllocatedRectangularCuboid>();
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

TEST_CASE("on", "[AllocatedRectangularCuboid][Geometry]") {
    auto const cuboid = AllocatedRectangularCuboid({3, 5, 7}, {11, 13, 17});
    SECTION("not on itself") {
        CHECK_FALSE(cuboid.on(cuboid));
    }
    SECTION("is on other") {
        CHECK(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 13, 0})));
        CHECK(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {13, 13, 0})));
        CHECK(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 17, 0})));
    }
    SECTION("touch only on a point") {
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {14, 13, 0})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 18, 0})));
    }
    SECTION("miss in one axis") {
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {23, 13, 0})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 45, 0})));
    }
    SECTION("other is below") {
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 16}, {11, 13, 0})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 16}, {13, 13, 0})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 16}, {11, 17, 0})));
    }
    SECTION("other overlaps") {
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 13, 1})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {13, 13, 1})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 17}, {11, 17, 1})));
    }
    SECTION("other above") {
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 7}, {11, 13, 25})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 7}, {13, 13, 25})));
        CHECK_FALSE(cuboid.on(AllocatedRectangularCuboid({3, 5, 7}, {11, 17, 25})));
    }
}

TEST_CASE("projection", "[AllocatedRectangularCuboid][Geometry]") {
    auto const cuboid = AllocatedRectangularCuboid({3, 5, 7}, {11, 13, 17});
    CHECK(cuboid.projection(Plane::XY) == Area::from_measurement(Plane::XY, {3, 5}, {11, 13}, 0));
    CHECK(cuboid.projection(Plane::XZ) == Area::from_measurement(Plane::XZ, {3, 7}, {11, 17}, 0));
    CHECK(cuboid.projection(Plane::YZ) == Area::from_measurement(Plane::YZ, {5, 7}, {13, 17}, 0));
}

TEST_CASE("face", "[AllocatedRectangularCuboid][Geometry]") {
    auto const cuboid = AllocatedRectangularCuboid({3, 5, 7}, {11, 13, 17});
    CHECK(cuboid.face(Face::BACK) == Area::from_measurement(Plane::YZ, {5, 7}, {13, 17}, 11));
    CHECK(cuboid.face(Face::FRONT) == Area::from_measurement(Plane::YZ, {5, 7}, {13, 17}, 14));
    CHECK(cuboid.face(Face::RIGHT) == Area::from_measurement(Plane::XZ, {3, 7}, {11, 17}, 13));
    CHECK(cuboid.face(Face::LEFT) == Area::from_measurement(Plane::XZ, {3, 7}, {11, 17}, 18));
    CHECK(cuboid.face(Face::BOTTOM) == Area::from_measurement(Plane::XY, {3, 5}, {11, 13}, 17));
    CHECK(cuboid.face(Face::TOP) == Area::from_measurement(Plane::XY, {3, 5}, {11, 13}, 24));
}

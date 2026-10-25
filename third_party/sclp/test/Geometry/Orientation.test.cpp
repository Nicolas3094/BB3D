#include <catch.hpp>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"

using namespace sclp;

TEST_CASE("EnabledOrientations - case 1", "[EnabledOrientations][Geometry]") {
    auto orientation_list = std::vector<OrientationType>{
        OrientationType::XYZ,
        OrientationType::ZYX,
        OrientationType::XZY,
    };
    auto enabled_orientations = EnabledOrientations(orientation_list);
    SECTION("check boolean values") {
        CHECK(enabled_orientations[OrientationType::XYZ] == true);
        CHECK(enabled_orientations[OrientationType::YXZ] == false);
        CHECK(enabled_orientations[OrientationType::ZYX] == true);
        CHECK(enabled_orientations[OrientationType::XZY] == true);
        CHECK(enabled_orientations[OrientationType::ZXY] == false);
        CHECK(enabled_orientations[OrientationType::YZX] == false);
    }
    SECTION("check iterator generator") {
        CHECK(std::vector<OrientationType>(enabled_orientations.begin(), enabled_orientations.end()) == orientation_list);
    }
    SECTION("check size") {
        CHECK(enabled_orientations.size() == 3);
    }
    SECTION("equality") {
        CHECK(enabled_orientations == enabled_orientations);
        CHECK_FALSE(enabled_orientations == EnabledOrientations());
    }
    SECTION("string") {
        CHECK(enabled_orientations.to_string() == "101100");
        CHECK(enabled_orientations.to_string(true) == "{xyz: true, yxz: false, zyx: true, xzy: true, zxy: false, yzx: false}");
    }
}

TEST_CASE("EnabledOrientations - case 2", "[EnabledOrientations][Geometry]") {
    auto orientation_list = std::vector<OrientationType>{
        OrientationType::YXZ,
        OrientationType::ZXY,
    };
    auto enabled_orientations = EnabledOrientations(orientation_list);
    SECTION("check boolean values") {
        CHECK(enabled_orientations[OrientationType::XYZ] == false);
        CHECK(enabled_orientations[OrientationType::YXZ] == true);
        CHECK(enabled_orientations[OrientationType::ZYX] == false);
        CHECK(enabled_orientations[OrientationType::XZY] == false);
        CHECK(enabled_orientations[OrientationType::ZXY] == true);
        CHECK(enabled_orientations[OrientationType::YZX] == false);
    }
    SECTION("check iterator generator") {
        CHECK(std::vector<OrientationType>(enabled_orientations.begin(), enabled_orientations.end()) == orientation_list);
    }
    SECTION("check size") {
        CHECK(enabled_orientations.size() == 2);
    }
}

TEST_CASE("EnabledOrientations - case 3", "[EnabledOrientations][Geometry]") {
    auto orientation_list = std::vector<OrientationType>{};
    auto enabled_orientations = EnabledOrientations(orientation_list);
    SECTION("check boolean values") {
        CHECK(enabled_orientations[OrientationType::XYZ] == false);
        CHECK(enabled_orientations[OrientationType::YXZ] == false);
        CHECK(enabled_orientations[OrientationType::ZYX] == false);
        CHECK(enabled_orientations[OrientationType::XZY] == false);
        CHECK(enabled_orientations[OrientationType::ZXY] == false);
        CHECK(enabled_orientations[OrientationType::YZX] == false);
    }
    SECTION("check iterator generator") {
        CHECK(std::vector<OrientationType>(enabled_orientations.begin(), enabled_orientations.end()) == orientation_list);
    }
    SECTION("check size") {
        CHECK(enabled_orientations.size() == 0);
    }
}

TEST_CASE("rotate", "[EnabledOrientations][Geometry]") {
    auto vector = DiscreteVector{1, 2, 3};
    CHECK(rotate(vector, OrientationType::XYZ) == vector);
    CHECK(rotate(vector, OrientationType::YXZ) == DiscreteVector{2, 1, 3});
    CHECK(rotate(vector, OrientationType::ZYX) == DiscreteVector{3, 2, 1});
    CHECK(rotate(vector, OrientationType::XZY) == DiscreteVector{1, 3, 2});
    CHECK(rotate(vector, OrientationType::ZXY) == DiscreteVector{3, 1, 2});
    CHECK(rotate(vector, OrientationType::YZX) == DiscreteVector{2, 3, 1});
    CHECK_THROWS(rotate(vector, OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS));
}

TEST_CASE("all values", "[EnabledOrientations][Geometry]") {
    CHECK(ORIENTATION_TYPE_ALL_VALUES.size() == static_cast<std::size_t>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS));
}

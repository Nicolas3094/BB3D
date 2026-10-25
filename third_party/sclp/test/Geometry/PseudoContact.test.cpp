#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/PseudoContact.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Packing/Packing.hpp"

using namespace sclp;

TEST_CASE("constructor throws", "[PseudoContact]") {
    auto const relative_tolerance = -1.0;
    CHECK_THROWS(PseudoContact(relative_tolerance));
}

TEST_CASE("constructor", "[PseudoContact][Geometry]") {
    auto const relative_tolerance = 0.5;
    auto const pseudo_contact = PseudoContact(relative_tolerance);
    CHECK(pseudo_contact.relative_tolerance() == Catch::Approx(relative_tolerance));
}

TEST_CASE("compute_absolute_tolerance", "[PseudoContact][Geometry]") {
    auto const relative_tolerance = 0.2;
    auto const pseudo_contact = PseudoContact(relative_tolerance);
    REQUIRE(pseudo_contact.relative_tolerance() == Catch::Approx(relative_tolerance));
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::BACK, {83, 77, 11}) == 16);
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::FRONT, {83, 77, 11}) == 16);
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::RIGHT, {83, 77, 11}) == 15);
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::LEFT, {83, 77, 11}) == 15);
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::BOTTOM, {83, 77, 11}) == 2);
    CHECK(pseudo_contact.compute_absolute_tolerance(Face::TOP, {83, 77, 11}) == 2);
}

TEST_CASE("compute_opposite_face", "[PseudoContact][Geometry]") {
    CHECK(PseudoContact::compute_opposite_face(Face::BACK) == Face::FRONT);
    CHECK(PseudoContact::compute_opposite_face(Face::FRONT) == Face::BACK);
    CHECK(PseudoContact::compute_opposite_face(Face::RIGHT) == Face::LEFT);
    CHECK(PseudoContact::compute_opposite_face(Face::LEFT) == Face::RIGHT);
    CHECK(PseudoContact::compute_opposite_face(Face::BOTTOM) == Face::TOP);
    CHECK(PseudoContact::compute_opposite_face(Face::TOP) == Face::BOTTOM);
}

TEST_CASE("compute_face_of", "[PseudoContact][Geometry]") {
    auto const small_item = SmallItem(0, 1, {3, 5, 7});
    auto block = std::make_shared<Block>();
    block->add(small_item, {0, 0, 0}, OrientationType::XYZ);
    auto const block_and_position = std::make_tuple(block, DiscreteVector{11, 13, 17});
    CHECK(PseudoContact::compute_face_of(Face::BACK, block_and_position) == Area::from_measurement(Plane::YZ, {5, 7}, {13, 17}, 11));
    CHECK(PseudoContact::compute_face_of(Face::FRONT, block_and_position) == Area::from_measurement(Plane::YZ, {5, 7}, {13, 17}, 14));
    CHECK(PseudoContact::compute_face_of(Face::RIGHT, block_and_position) == Area::from_measurement(Plane::XZ, {3, 7}, {11, 17}, 13));
    CHECK(PseudoContact::compute_face_of(Face::LEFT, block_and_position) == Area::from_measurement(Plane::XZ, {3, 7}, {11, 17}, 18));
    CHECK(PseudoContact::compute_face_of(Face::BOTTOM, block_and_position) == Area::from_measurement(Plane::XY, {3, 5}, {11, 13}, 17));
    CHECK(PseudoContact::compute_face_of(Face::TOP, block_and_position) == Area::from_measurement(Plane::XY, {3, 5}, {11, 13}, 24));
}

TEST_CASE("find_areas", "[PseudoContact][Geometry]") {
    // initialize data
    auto const small_items = std::vector{
        SmallItem{0, 9, {72, 40, 42}},
        SmallItem{1, 9, {53, 74, 81}},
        SmallItem{2, 9, {30, 70, 19}},
        SmallItem{3, 9, {40, 73, 27}},
    };
    auto singular_blocks = std::vector<Block::Sptr>{};
    for (auto const & small_item : small_items) {
        singular_blocks.push_back(std::make_shared<Block>());
        singular_blocks.back()->add(small_item, DiscreteVector::zeros(), OrientationType::XYZ);
    }
    auto const empty_space = EmptySpace({32, 49, 68}, {100, 110, 120});
    auto packing = Packing();
    auto const relative_tolerance = 0.2;
    auto const pseudo_contact = PseudoContact(relative_tolerance);
    REQUIRE(pseudo_contact.relative_tolerance() == Catch::Approx(relative_tolerance));
    // BACK
    packing.add(singular_blocks[0], {28, 80, 90});
    packing.add(singular_blocks[1], {44, 140, 150});
    packing.add(singular_blocks[2], {64, 60, 140});
    auto const back_areas = std::vector<Area>{
        Area::from_measurement(Plane::YZ, {10, 12}, {110, 120}, 100),
        Area::from_measurement(Plane::YZ, {19, 38}, {140, 150}, 97),
        Area::from_measurement(Plane::YZ, {20, 19}, {110, 140}, 94),
    };
    // FRONT
    auto const front_areas = std::vector<Area>{};
    // RIGHT
    auto const right_areas = std::vector<Area>{};
    // LEFT
    auto const left_areas = std::vector<Area>{};
    // BOTTOM
    auto const bottom_areas = std::vector<Area>{};
    // TOP
    auto const top_areas = std::vector<Area>{};
    // check
    auto const expected_areas = std::array<std::vector<Area>, NUMBER_OF_FACES>{{
        back_areas,
        front_areas,
        right_areas,
        left_areas,
        bottom_areas,
        top_areas,
    }};
    auto const actual_areas = pseudo_contact.find_areas(empty_space, packing);
    CHECK(actual_areas == expected_areas);
}

TEST_CASE("compute_contact_area", "[PseudoContact][Geometry]") {
    auto const relative_tolerance = 0.2;
    auto const pseudo_contact = PseudoContact(relative_tolerance);
    auto const rectangular_cuboid = sclp::AllocatedRectangularCuboid({30, 41, 55}, {53, 36, 42});
    auto const back_areas = std::vector<Area>{};
    // FRONT
    auto const front_areas = std::vector<Area>{
        // x tolerance: 6
        Area::from_positions(Plane::YZ, {41, 71}, {63, 128}, 84),
        Area::from_positions(Plane::YZ, {52, 27}, {89, 68}, 90), // x too high
    };
    // RIGHT
    auto const right_areas = std::vector<Area>{
        // y tolerance: 8
        Area::from_positions(Plane::XZ, {71, 21}, {94, 64}, 41),
        Area::from_positions(Plane::XZ, {54, 86}, {59, 95}, 36),
        Area::from_positions(Plane::XZ, {55, 50}, {63, 70}, 20), // y too low
    };
    // LEFT
    auto const left_areas = std::vector<Area>{
        // y tolerance: 8
        Area::from_positions(Plane::XZ, {64, 64}, {72, 75}, 77), // y too high
    };
    // BOTTOM
    auto const bottom_areas = std::vector<Area>{
        // z tolerance: 11
        Area::from_positions(Plane::XY, {30, 12}, {110, 120}, 31),
    };
    // TOP
    auto const top_areas = std::vector<Area>{
        Area::from_positions(Plane::XY, {20, 19}, {99, 56}, 108),
    };
    // check
    auto const areas = std::array<std::vector<Area>, NUMBER_OF_FACES>{{
        back_areas,
        front_areas,
        right_areas,
        left_areas,
        bottom_areas,
        top_areas,
    }};
    auto const contact_area = pseudo_contact.compute_contact_area(areas, rectangular_cuboid);
    auto const expected = 572 // front
        + 0 // back
        + 309 // right
        + 88 // left
        + 1230 // bottom
        + 600 // top
        ;
    CHECK(contact_area == expected);
}

TEST_CASE("add_contact_areas_with_large_object", "[PseudoContact][Geometry]") {
    auto const large_object = sclp::RectangularCuboid({100, 200, 300});
    auto const empty_space = EmptySpace({98, 196, 294}, {1, 2, 3});
    auto const relative_tolerance = 0.2;
    auto const pseudo_contact = PseudoContact(relative_tolerance);
    auto areas = std::array<std::vector<Area>, NUMBER_OF_FACES>();
    pseudo_contact.add_contact_areas_with_large_object(empty_space, large_object, areas);
    auto const expected = std::array<std::vector<Area>, NUMBER_OF_FACES>{{
        std::vector<Area>{Area::from_measurement(Plane::YZ, {196, 294}, {2, 3}, 1)}, // Face::BACK
        std::vector<Area>{Area::from_measurement(Plane::YZ, {196, 294}, {2, 3}, 99)}, // Face::FRONT
        std::vector<Area>{Area::from_measurement(Plane::XZ, {98, 294}, {1, 3}, 2)}, // Face::RIGHT
        std::vector<Area>{Area::from_measurement(Plane::XZ, {98, 294}, {1, 3}, 198)}, // Face::LEFT
        std::vector<Area>{Area::from_measurement(Plane::XY, {98, 196}, {1, 2}, 3)}, // Face::BOTTOM
        std::vector<Area>{Area::from_measurement(Plane::XY, {98, 196}, {1, 2}, 297)}, // Face::TOP
    }};
    CHECK(areas == expected);
}

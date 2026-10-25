#include <catch.hpp>

#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

TEST_CASE("constructor", "[DiscreteVector][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const z = CoordinateType{5};
    auto const v = DiscreteVector{x, y, z};
    CHECK(v.x() == x);
    CHECK(v.y() == y);
    CHECK(v.z() == z);
    CHECK(v.get<0>() == x);
    CHECK(v.get<1>() == y);
    CHECK(v.get<2>() == z);
    CHECK(v.get<Axis::X>() == x);
    CHECK(v.get<Axis::Y>() == y);
    CHECK(v.get<Axis::Z>() == z);
    CHECK(v.get(Axis::X) == x);
    CHECK(v.get(Axis::Y) == y);
    CHECK(v.get(Axis::Z) == z);
    CHECK(v.prod() == 30);
    CHECK(v.norm() == Catch::Approx(6.164414003));
}

TEST_CASE("equality", "[DiscreteVector][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const z = CoordinateType{5};
    auto const v = DiscreteVector{x, y, z};
    SECTION("true") {
        CHECK(v == DiscreteVector{x, y, z});
    }
    SECTION("false") {
        CHECK_FALSE(v == DiscreteVector{x + 1, y, z});
        CHECK_FALSE(v == DiscreteVector{x, y + 1, z});
        CHECK_FALSE(v == DiscreteVector{x, y, z + 1});
    }
}

TEST_CASE("sum", "[DiscreteVector][Geometry]") {
    auto const x_lhs = CoordinateType{2};
    auto const y_lhs = CoordinateType{3};
    auto const z_lhs = CoordinateType{5};
    auto const lhs = DiscreteVector{x_lhs, y_lhs, z_lhs};
    auto const x_rhs = CoordinateType{7};
    auto const y_rhs = CoordinateType{11};
    auto const z_rhs = CoordinateType{13};
    auto const rhs = DiscreteVector{x_rhs, y_rhs, z_rhs};
    auto const expected = DiscreteVector{x_lhs + x_rhs, y_lhs + y_rhs, z_lhs + z_rhs};
    CHECK((lhs + rhs) == expected);
}

TEST_CASE("subtraction", "[DiscreteVector][Geometry]") {
    auto const x_lhs = CoordinateType{2};
    auto const y_lhs = CoordinateType{3};
    auto const z_lhs = CoordinateType{5};
    auto const lhs = DiscreteVector{x_lhs, y_lhs, z_lhs};
    auto const x_rhs = CoordinateType{7};
    auto const y_rhs = CoordinateType{11};
    auto const z_rhs = CoordinateType{13};
    auto const rhs = DiscreteVector{x_rhs, y_rhs, z_rhs};
    auto const expected = DiscreteVector{x_lhs - x_rhs, y_lhs - y_rhs, z_lhs - z_rhs};
    CHECK((lhs - rhs) == expected);
}

TEST_CASE("less", "[DiscreteVector][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const z = CoordinateType{5};
    auto const v = DiscreteVector{x, y, z};
    SECTION("true") {
        CHECK((v < DiscreteVector{x + 1, y + 1, z + 1}));
        CHECK((v < DiscreteVector{x + 9, y + 9, z + 9}));
        CHECK((DiscreteVector{x - 1, y - 1, z - 1} < v));
    }
    SECTION("false") {
        CHECK_FALSE((v < DiscreteVector{x + 0, y + 1, z + 1}));
        CHECK_FALSE((v < DiscreteVector{x + 1, y + 0, z + 1}));
        CHECK_FALSE((v < DiscreteVector{x + 1, y + 1, z + 0}));
        CHECK_FALSE((v < DiscreteVector{x + 0, y + 0, z + 1}));
        CHECK_FALSE((v < DiscreteVector{x + 0, y + 1, z + 0}));
        CHECK_FALSE((v < DiscreteVector{x + 1, y + 0, z + 0}));
        CHECK_FALSE((v < v));
    }
}

TEST_CASE("lexicographical less", "[DiscreteVector][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const z = CoordinateType{5};
    auto const v = DiscreteVector{x, y, z};
    SECTION("true") {
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 1, y + 1, z + 1}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 9, y + 9, z + 9}));
        CHECK(DiscreteVector::lexicographical_less(DiscreteVector{x - 1, y - 1, z - 1}, v));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y + 1, z + 1}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y + 0, z + 1}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y + 1, z + 0}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 1, y + 0, z + 1}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 1, y + 1, z + 0}));
        CHECK(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 1, y + 0, z + 0}));
    }
    SECTION("false I") {
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, v));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y + 0, z + 0}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y - 1, z + 0}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y + 0, z - 1}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y - 1, z + 0}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y + 0, z - 1}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y - 1, z - 1}));
    }
    SECTION("false II") {
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, v));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y + 9, z + 9}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y - 1, z + 9}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y + 0, z - 1}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y - 1, z + 9}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x - 1, y + 9, z - 1}));
        CHECK_FALSE(DiscreteVector::lexicographical_less(v, DiscreteVector{x + 0, y - 1, z - 1}));
    }
}

TEST_CASE("lexicographical", "[DiscreteVector][Geometry]") {
    // test all permutations
    CHECK(DiscreteVector{2, 3, 5}.lexicographical() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{3, 2, 5}.lexicographical() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{5, 3, 2}.lexicographical() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{2, 5, 3}.lexicographical() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{5, 2, 3}.lexicographical() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{3, 5, 2}.lexicographical() == DiscreteVector{2, 3, 5});
}

TEST_CASE("abs", "[DiscreteVector][Geometry]") {
    // test all permutations
    CHECK(DiscreteVector{-2, 3, 5}.abs() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{2, -3, 5}.abs() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{2, 3, -5}.abs() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{-2, -3, 5}.abs() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{-2, 3, -5}.abs() == DiscreteVector{2, 3, 5});
    CHECK(DiscreteVector{2, -3, -5}.abs() == DiscreteVector{2, 3, 5});
}

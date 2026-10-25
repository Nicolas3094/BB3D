#include <catch.hpp>

#include "sclp/Geometry/DiscreteVector.hpp"

using namespace sclp;

TEST_CASE("constructor", "[DiscreteVector2D][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const v = DiscreteVector2D{x, y};
    CHECK(v.x() == x);
    CHECK(v.get<0>() == x);
    CHECK(v.get<1>() == y);
    CHECK(v.y() == y);
    CHECK(v.prod() == 6);
    CHECK(v.dot(v) == 13);
    CHECK(v.cross(v) == 0);
    CHECK(v.squaredNorm() == 13);
    CHECK(v.norm() == Catch::Approx(3.605551275463989));
}

TEST_CASE("equality", "[DiscreteVector2D][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const v = DiscreteVector2D{x, y};
    SECTION("true") {
        CHECK(v == DiscreteVector2D{x, y});
    }
    SECTION("false") {
        CHECK_FALSE(v == DiscreteVector2D{x + 1, y});
        CHECK_FALSE(v == DiscreteVector2D{x, y + 1});
    }
}

TEST_CASE("cross", "[DiscreteVector2D][Geometry]") {
    auto const lhs = DiscreteVector2D{2, 3};
    auto const rhs = DiscreteVector2D{7, 11};
    CHECK(lhs.cross(rhs) == 1);
}

TEST_CASE("sum", "[DiscreteVector2D][Geometry]") {
    auto const x_lhs = CoordinateType{2};
    auto const y_lhs = CoordinateType{3};
    auto const lhs = DiscreteVector2D{x_lhs, y_lhs};
    auto const x_rhs = CoordinateType{7};
    auto const y_rhs = CoordinateType{11};
    auto const rhs = DiscreteVector2D{x_rhs, y_rhs};
    auto const expected = DiscreteVector2D{x_lhs + x_rhs, y_lhs + y_rhs};
    CHECK((lhs + rhs) == expected);
}

TEST_CASE("subtraction", "[DiscreteVector2D][Geometry]") {
    auto const x_lhs = CoordinateType{2};
    auto const y_lhs = CoordinateType{3};
    auto const lhs = DiscreteVector2D{x_lhs, y_lhs};
    auto const x_rhs = CoordinateType{7};
    auto const y_rhs = CoordinateType{11};
    auto const rhs = DiscreteVector2D{x_rhs, y_rhs};
    auto const expected = DiscreteVector2D{x_lhs - x_rhs, y_lhs - y_rhs};
    CHECK((lhs - rhs) == expected);
}

TEST_CASE("less", "[DiscreteVector2D][Geometry]") {
    auto const x = CoordinateType{2};
    auto const y = CoordinateType{3};
    auto const v = DiscreteVector2D{x, y};
    SECTION("true") {
        CHECK((v < DiscreteVector2D{x + 1, y + 1}));
        CHECK((v < DiscreteVector2D{x + 9, y + 9}));
        CHECK((DiscreteVector2D{x - 1, y - 1} < v));
    }
    SECTION("false") {
        CHECK_FALSE((v < v));
        CHECK_FALSE((v < DiscreteVector2D{x + 0, y + 1}));
        CHECK_FALSE((v < DiscreteVector2D{x + 1, y + 0}));
    }
}

#include <catch.hpp>

#include "sclp/Geometry/PlaneVector.hpp"

using namespace sclp;

auto check_approx(PlaneVector const & lhs, PlaneVector const & rhs) -> void {
    CHECK(((lhs.x() == Catch::Approx(rhs.x())) && (lhs.y() == Catch::Approx(rhs.y()))));
    return;
}

auto check_false_approx(PlaneVector const & lhs, PlaneVector const & rhs) -> void {
    CHECK(((lhs.x() != Catch::Approx(rhs.x())) || (lhs.y() != Catch::Approx(rhs.y()))));
    return;
}

TEST_CASE("constructor", "[PlaneVector][Geometry]") {
    auto const x = PlaneVector::scalar{2.0};
    auto const y = PlaneVector::scalar{3.0};
    auto const v = PlaneVector{x, y};
    CHECK(v.x() == Catch::Approx(x));
    CHECK(v.y() == Catch::Approx(y));
    CHECK(v.get<0>() == Catch::Approx(x));
    CHECK(v.get<1>() == Catch::Approx(y));
    CHECK(v.prod() == Catch::Approx(6.0));
    CHECK(v.norm() == Catch::Approx(std::sqrt(13.0)));
}

TEST_CASE("equality", "[PlaneVector][Geometry]") {
    auto const x = PlaneVector::scalar{2.0};
    auto const y = PlaneVector::scalar{3.0};
    auto const v = PlaneVector{x, y};
    SECTION("true") {
        check_approx(v, PlaneVector{x, y});
    }
    SECTION("false") {
        check_false_approx(v, PlaneVector{x + 1, y});
        check_false_approx(v, PlaneVector{x, y + 1});
    }
}

TEST_CASE("sum", "[PlaneVector][Geometry]") {
    auto const x_lhs = PlaneVector::scalar{2.0};
    auto const y_lhs = PlaneVector::scalar{3.0};
    auto const lhs = PlaneVector{x_lhs, y_lhs};
    auto const x_rhs = PlaneVector::scalar{7.0};
    auto const y_rhs = PlaneVector::scalar{11.0};
    auto const rhs = PlaneVector{x_rhs, y_rhs};
    auto const expected = PlaneVector{x_lhs + x_rhs, y_lhs + y_rhs};
    check_approx((lhs + rhs), expected);
}

TEST_CASE("subtraction", "[PlaneVector][Geometry]") {
    auto const x_lhs = PlaneVector::scalar{2.0};
    auto const y_lhs = PlaneVector::scalar{3.0};
    auto const lhs = PlaneVector{x_lhs, y_lhs};
    auto const x_rhs = PlaneVector::scalar{7.0};
    auto const y_rhs = PlaneVector::scalar{11.0};
    auto const rhs = PlaneVector{x_rhs, y_rhs};
    auto const expected = PlaneVector{x_lhs - x_rhs, y_lhs - y_rhs};
    check_approx((lhs - rhs), expected);
}

TEST_CASE("cross", "[PlaneVector][Geometry]") {
    auto const u = PlaneVector{3, 7};
    auto const v = PlaneVector{5, 11};
    CHECK(u.cross(v) == Catch::Approx(-2.0));
}

TEST_CASE("less", "[PlaneVector][Geometry]") {
    auto const x = PlaneVector::scalar{2.0};
    auto const y = PlaneVector::scalar{3.0};
    auto const v = PlaneVector{x, y};
    SECTION("true") {
        CHECK((v < PlaneVector{x + 1.0, y + 1.0}));
        CHECK((v < PlaneVector{x + 9.0, y + 9.0}));
        CHECK((PlaneVector{x - 1.0, y - 1.0} < v));
    }
    SECTION("false") {
        CHECK_FALSE((v < PlaneVector{x + 0.0, y + 1.0}));
        CHECK_FALSE((v < PlaneVector{x + 1.0, y + 0.0}));
        CHECK_FALSE((v < v));
    }
}

TEST_CASE("convex hull", "[PlaneVector][Geometry]") {
    SECTION("exception cases") {
        SECTION("case 0") {
            auto points = std::vector<PlaneVector>{};
            PlaneVector::convex_hull(points);
            auto const expected = std::vector<PlaneVector>{};
            CHECK(points == expected);
        }
        SECTION("case 1") {
            auto points = std::vector<PlaneVector>{
                {0, 0},
            };
            PlaneVector::convex_hull(points);
            auto const expected = std::vector<PlaneVector>{
                {0, 0},
            };
            CHECK(points == expected);
        }
        SECTION("case 2") {
            auto points = std::vector<PlaneVector>{
                {0, 0},
                {0, 1},
            };
            PlaneVector::convex_hull(points);
            auto const expected = std::vector<PlaneVector>{
                {0, 1},
                {0, 0},
            };
            CHECK(points == expected);
        }
    }
    SECTION("common cases") {
        SECTION("case 0") {
            auto points = std::vector<PlaneVector>{
                {0, 0},
                {10, 0},
                {0, 10},
                {10, 10},
                {8, 3},
                {4, 3},
                {4, 2},
                {4, 3},
                {9, 1},
                {8, 1},
                {3, 1},
                {8, 2},
                {3, 6},
                {7, 6},
                {3, 9},
                {3, 5},
                {7, 9},
                {3, 7},
                {8, 1},
            };
            PlaneVector::convex_hull(points);
            auto const expected = std::vector<PlaneVector>{
                {10, 0},
                {10, 10},
                {0, 10},
                {0, 0},
            };
            CHECK(points == expected);
        }
        SECTION("case 1") {
            auto points = std::vector<PlaneVector>{
                {41, -6},
                {-24, -74},
                {-51, -6},
                {73, 17},
                {-30, -34},
            };
            PlaneVector::convex_hull(points);
            auto const expected = std::vector<PlaneVector>{
                {-24, -74},
                {73, 17},
                {-51, -6},
            };
            CHECK(points == expected);
        }
    }
}

TEST_CASE("angle", "[PlaneVector][Geometry]") {
    SECTION("case 0") {
        auto const first = PlaneVector{1, 0};
        auto const pivot = PlaneVector{0, 0};
        auto const second = PlaneVector{0, 1};
        CHECK(PlaneVector::angle(first, pivot, second) == Catch::Approx(M_PI / 2.0));
        CHECK(PlaneVector::angle(second, pivot, first) == Catch::Approx(M_PI / 2.0));
    }
    SECTION("case 1") {
        auto const first = PlaneVector{0, -1};
        auto const pivot = PlaneVector{0, 0};
        auto const second = PlaneVector{0, 1};
        CHECK(PlaneVector::angle(first, pivot, second) == Catch::Approx(M_PI));
        CHECK(PlaneVector::angle(second, pivot, first) == Catch::Approx(M_PI));
    }
    SECTION("case 2") {
        auto const first = PlaneVector{1, 0};
        auto const pivot = PlaneVector{0, 0};
        auto const second = PlaneVector{std::sqrt(2) / 2, std::sqrt(2) / 2};
        CHECK(PlaneVector::angle(first, pivot, second) == Catch::Approx(M_PI / 4));
        CHECK(PlaneVector::angle(second, pivot, first) == Catch::Approx(M_PI / 4));
    }
}

TEST_CASE("point in polygon", "[PlaneVector][Geometry]") {
    auto const polygon = std::vector<PlaneVector>{
        {3, 1},
        {3, 3},
        {5, 3},
        {5, 1},
        {7, 1},
        {7, 9},
        {5, 9},
        {5, 5},
        {3, 5},
        {3, 9},
        {1, 9},
        {1, 1},
    };
    SECTION("true") {
        CHECK(PlaneVector::insidePolygon({1, 1}, polygon)); // on vertex
        CHECK(PlaneVector::insidePolygon({7, 3}, polygon)); // on edge
        CHECK(PlaneVector::insidePolygon({6.99, 1.01}, polygon)); // interior
        CHECK(PlaneVector::insidePolygon({4, 4}, polygon)); // interior
        CHECK(PlaneVector::insidePolygon({6, 4}, polygon)); // interior
        CHECK(PlaneVector::insidePolygon({6, 8}, polygon)); // interior
    }
    SECTION("false") {
        CHECK_FALSE(PlaneVector::insidePolygon({0, 0}, polygon));
        CHECK_FALSE(PlaneVector::insidePolygon({10, 10}, polygon));
        CHECK_FALSE(PlaneVector::insidePolygon({0.9, 1}, polygon));
        CHECK_FALSE(PlaneVector::insidePolygon({1, 0.9}, polygon));
        CHECK_FALSE(PlaneVector::insidePolygon({7.01, 1.01}, polygon));
    }
}

#include <catch.hpp>

#include "sclp/Geometry/ContinuousVector.hpp"

using namespace sclp;

auto check_approx(ContinuousVector const & lhs, ContinuousVector const & rhs) -> void {
    CHECK(((lhs.x() == Catch::Approx(rhs.x())) && (lhs.y() == Catch::Approx(rhs.y())) && (lhs.z() == Catch::Approx(rhs.z()))));
    return;
}

auto check_false_approx(ContinuousVector const & lhs, ContinuousVector const & rhs) -> void {
    CHECK(((lhs.x() != Catch::Approx(rhs.x())) || (lhs.y() != Catch::Approx(rhs.y())) || (lhs.z() != Catch::Approx(rhs.z()))));
    return;
}

TEST_CASE("constructor", "[ContinuousVector][Geometry]") {
    auto const x = DistanceType{2.0};
    auto const y = DistanceType{3.0};
    auto const z = DistanceType{5.0};
    auto const v = ContinuousVector{x, y, z};
    CHECK(v.x() == Catch::Approx(x));
    CHECK(v.y() == Catch::Approx(y));
    CHECK(v.z() == Catch::Approx(z));
    CHECK(v.get<0>() == Catch::Approx(x));
    CHECK(v.get<1>() == Catch::Approx(y));
    CHECK(v.get<2>() == Catch::Approx(z));
    CHECK(v.get<Axis::X>() == Catch::Approx(x));
    CHECK(v.get<Axis::Y>() == Catch::Approx(y));
    CHECK(v.get<Axis::Z>() == Catch::Approx(z));
    CHECK(v.get(Axis::X) == Catch::Approx(x));
    CHECK(v.get(Axis::Y) == Catch::Approx(y));
    CHECK(v.get(Axis::Z) == Catch::Approx(z));
    CHECK(v.prod() == Catch::Approx(30.0));
    CHECK(v.norm() == Catch::Approx(std::sqrt(38)));
}

TEST_CASE("equality", "[ContinuousVector][Geometry]") {
    auto const x = DistanceType{2.0};
    auto const y = DistanceType{3.0};
    auto const z = DistanceType{5.0};
    auto const v = ContinuousVector{x, y, z};
    SECTION("true") {
        CHECK(v == v);
    }
    SECTION("false") {
        CHECK_FALSE(v == ContinuousVector{x + 1, y, z});
        CHECK_FALSE(v == ContinuousVector{x, y + 1, z});
        CHECK_FALSE(v == ContinuousVector{x, y, z + 1});
    }
}

TEST_CASE("sum", "[ContinuousVector][Geometry]") {
    auto const x_lhs = DistanceType{2.0};
    auto const y_lhs = DistanceType{3.0};
    auto const z_lhs = DistanceType{5.0};
    auto const lhs = ContinuousVector{x_lhs, y_lhs, z_lhs};
    auto const x_rhs = DistanceType{7.0};
    auto const y_rhs = DistanceType{11.0};
    auto const z_rhs = DistanceType{13.0};
    auto const rhs = ContinuousVector{x_rhs, y_rhs, z_rhs};
    auto const expected = ContinuousVector{x_lhs + x_rhs, y_lhs + y_rhs, z_lhs + z_rhs};
    check_approx((lhs + rhs), expected);
}

TEST_CASE("subtraction", "[ContinuousVector][Geometry]") {
    auto const x_lhs = DistanceType{2.0};
    auto const y_lhs = DistanceType{3.0};
    auto const z_lhs = DistanceType{5.0};
    auto const lhs = ContinuousVector{x_lhs, y_lhs, z_lhs};
    auto const x_rhs = DistanceType{7.0};
    auto const y_rhs = DistanceType{11.0};
    auto const z_rhs = DistanceType{13.0};
    auto const rhs = ContinuousVector{x_rhs, y_rhs, z_rhs};
    auto const expected = ContinuousVector{x_lhs - x_rhs, y_lhs - y_rhs, z_lhs - z_rhs};
    check_approx((lhs - rhs), expected);
}

TEST_CASE("less", "[ContinuousVector][Geometry]") {
    auto const x = DistanceType{2.0};
    auto const y = DistanceType{3.0};
    auto const z = DistanceType{5.0};
    auto const v = ContinuousVector{x, y, z};
    SECTION("true") {
        CHECK((v < ContinuousVector{x + 1.0, y + 1.0, z + 1.0}));
        CHECK((v < ContinuousVector{x + 9.0, y + 9.0, z + 9.0}));
        CHECK((ContinuousVector{x - 1.0, y - 1.0, z - 1.0} < v));
    }
    SECTION("false") {
        CHECK_FALSE((v < ContinuousVector{x + 0.0, y + 1.0, z + 1.0}));
        CHECK_FALSE((v < ContinuousVector{x + 1.0, y + 0.0, z + 1.0}));
        CHECK_FALSE((v < ContinuousVector{x + 1.0, y + 1.0, z + 0.0}));
        CHECK_FALSE((v < ContinuousVector{x + 0.0, y + 0.0, z + 1.0}));
        CHECK_FALSE((v < ContinuousVector{x + 0.0, y + 1.0, z + 0.0}));
        CHECK_FALSE((v < ContinuousVector{x + 1.0, y + 0.0, z + 0.0}));
        CHECK_FALSE((v < v));
    }
}

TEST_CASE("lexicographical less", "[ContinuousVector][Geometry]") {
    auto const x = DistanceType{2};
    auto const y = DistanceType{3};
    auto const z = DistanceType{5};
    auto const v = ContinuousVector{x, y, z};
    SECTION("true") {
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 1, y + 1, z + 1}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 9, y + 9, z + 9}));
        CHECK(ContinuousVector::lexicographical_less(ContinuousVector{x - 1, y - 1, z - 1}, v));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y + 1, z + 1}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y + 0, z + 1}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y + 1, z + 0}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 1, y + 0, z + 1}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 1, y + 1, z + 0}));
        CHECK(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 1, y + 0, z + 0}));
    }
    SECTION("false I") {
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, v));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y + 0, z + 0}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y - 1, z + 0}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y + 0, z - 1}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y - 1, z + 0}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y + 0, z - 1}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y - 1, z - 1}));
    }
    SECTION("false II") {
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, v));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y + 9, z + 9}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y - 1, z + 9}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y + 0, z - 1}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y - 1, z + 9}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x - 1, y + 9, z - 1}));
        CHECK_FALSE(ContinuousVector::lexicographical_less(v, ContinuousVector{x + 0, y - 1, z - 1}));
    }
}

TEST_CASE("lexicographical", "[ContinuousVector][Geometry]") {
    // test all permutations
    CHECK(ContinuousVector{2.81, 3.98, 5.46}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{3.98, 2.81, 5.46}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{5.46, 3.98, 2.81}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{2.81, 5.46, 3.98}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{5.46, 2.81, 3.98}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{3.98, 5.46, 2.81}.lexicographical() == ContinuousVector{2.81, 3.98, 5.46});
}

TEST_CASE("abs", "[ContinuousVector][Geometry]") {
    // test all permutations
    CHECK(ContinuousVector{-2.81, 3.98, 5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{2.81, -3.98, 5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{2.81, 3.98, -5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{-2.81, -3.98, 5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{-2.81, 3.98, -5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
    CHECK(ContinuousVector{2.81, -3.98, -5.46}.abs() == ContinuousVector{2.81, 3.98, 5.46});
}

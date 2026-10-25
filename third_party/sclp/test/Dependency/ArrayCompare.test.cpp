#include <catch.hpp>

#include <array>

TEST_CASE("array compare same size", "[std][Dependency]") {
    SECTION("equality") {
        CHECK(std::array{1, 2, 3} == std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} <= std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} <= std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} >= std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} >= std::array{1, 2, 3});
    }
    SECTION("inequality") {
        CHECK(std::array{1, 2, 3} != std::array{1, 2, 30});
    }
    SECTION("lower I") {
        CHECK(std::array{1, 2, 3} < std::array{10, 2, 3});
        CHECK(std::array{10, 2, 3} > std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} < std::array{1, 20, 3});
        CHECK(std::array{1, 20, 3} > std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} < std::array{1, 2, 30});
        CHECK(std::array{1, 2, 30} > std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} < std::array{10, 20, 3});
        CHECK(std::array{10, 20, 3} > std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} < std::array{10, 2, 30});
        CHECK(std::array{10, 2, 30} > std::array{1, 2, 3});
        CHECK(std::array{1, 2, 3} < std::array{1, 20, 30});
        CHECK(std::array{1, 20, 30} > std::array{1, 2, 3});
    }
    SECTION("lower II") {
        CHECK(std::array{0, 2, 3} < std::array{1, 2, 3});
        CHECK(std::array{1, 0, 3} < std::array{1, 2, 3});
        CHECK(std::array{1, 2, 0} < std::array{1, 2, 3});
        CHECK(std::array{0, 0, 3} < std::array{1, 2, 3});
        CHECK(std::array{0, 2, 0} < std::array{1, 2, 3});
        CHECK(std::array{1, 0, 0} < std::array{1, 2, 3});
    }
}

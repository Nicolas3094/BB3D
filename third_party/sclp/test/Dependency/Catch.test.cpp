#include <catch.hpp>

#include <vector>

TEST_CASE("Catch", "[Catch][Dependency]") {
    REQUIRE(1 == 1);
    CHECK(2 == 2);
    CHECK(2.00000000001 == Catch::Approx(2.0));
    std::vector<int> v{1, 2, 3};
    CHECK(v == std::vector<int>{1, 2, 3});
    REQUIRE_THAT(v, Catch::Matchers::UnorderedEquals(std::vector<int>{3, 2, 1}));
}

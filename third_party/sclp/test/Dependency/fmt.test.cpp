#include <catch.hpp>

#include <fmt/core.h>

TEST_CASE("fmt", "[fmt][Dependency]") {
    auto const s = fmt::format("The answer is {}.", 42);
    CHECK(s == "The answer is 42.");
    auto const t = fmt::format("The answer is {{{}}}.", 42);
    CHECK(t == "The answer is {42}.");
}

#include <catch.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

TEST_CASE("Json", "[Json]") {
    auto o = sclp::Json{};
    o["key"] = "value";
    CHECK(o.dump() == "{\"key\":\"value\"}");
}

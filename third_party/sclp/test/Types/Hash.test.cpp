#include <catch.hpp>

#include <unordered_set>
#include <vector>

#include "sclp/Types/Hash.hpp"

using namespace sclp;

TEST_CASE("constructor and conversion", "[Hash]") {
    auto h0 = Hash(42);
    REQUIRE(h0 != 42);

    auto h1 = Hash(31);
    REQUIRE(h1 != 31);

    auto h2 = Hash();

    CHECK(h0 != h1);
    CHECK(h0 != h2);
    CHECK(h1 != h2);
}

TEST_CASE("allow constructor from pointer", "[Hash]") {
    auto int_value = 56;
    auto h = std::vector<Hash>{
        reinterpret_cast<double *>(&int_value),
        reinterpret_cast<void *>(&int_value),
    };
    // check that all hashes are equal
    for (auto i = std::size_t(0); i < h.size(); ++i) {
        for (auto j = std::size_t(0); j < h.size(); ++j) {
            CHECK(h[i] == h[j]);
        }
    }
}

TEST_CASE("constructor from many values", "[Hash]") {
    auto h = std::vector<Hash>{
        {42, 31, 5},
        {42, 5, 31},
        {31, 42, 5},
        {31, 5, 42},
        {5, 31, 42},
        {5, 42, 31},
    };
    for (auto i = std::size_t(0); i < h.size(); ++i) {
        for (auto j = std::size_t(0); j < h.size(); ++j) {
            if (i == j) { continue; }
            CHECK(h[i] != h[j]);
        }
    }
}

TEST_CASE("combine unique", "[Hash]") {
    auto combinations_of_values = std::vector<std::vector<int>>{
        {42, 31, 5},
        {42, 5, 31},
        {31, 42, 5},
        {31, 5, 42},
        {5, 31, 42},
        {5, 42, 31},
    };
    // initialize hashes from values
    auto h = std::vector<Hash>();
    for (auto const & values : combinations_of_values) {
        h.push_back(Hash());
        for (auto const & v : values) {
            h.back().combine_unique(v);
        }
    }
    // check that all hashes are different
    for (auto i = std::size_t(0); i < h.size(); ++i) {
        for (auto j = std::size_t(0); j < h.size(); ++j) {
            if (i == j) { continue; }
            CHECK(h[i] != h[j]);
        }
    }
}

TEST_CASE("combine commutative", "[Hash]") {
    auto combinations_of_values = std::vector<std::vector<int>>{
        {42, 31, 5},
        {42, 5, 31},
        {31, 42, 5},
        {31, 5, 42},
        {5, 31, 42},
        {5, 42, 31},
    };
    // initialize hashes from values
    auto h = std::vector<Hash>();
    for (auto const & values : combinations_of_values) {
        h.push_back(Hash());
        for (auto const & v : values) {
            h.back().combine_commutative(v);
        }
    }
    // check that all hashes are equal
    for (auto i = std::size_t(0); i < h.size(); ++i) {
        for (auto j = std::size_t(0); j < h.size(); ++j) {
            CHECK(h[i] == h[j]);
        }
    }
}

TEST_CASE("unordered set", "[Hash]") {
    auto hashes = std::unordered_set<Hash>();
    hashes.insert(3);
    hashes.insert(7);
    CHECK(hashes.find(3) != hashes.end());
    CHECK(hashes.find(7) != hashes.end());
    CHECK(hashes.find(4) == hashes.end());
}

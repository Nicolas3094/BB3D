#include <catch.hpp>

#include <filesystem>

#include "sclp/IO/FileLoader.hpp"

auto const data_dir = std::filesystem::path("test/data/IO/");

using namespace sclp;

TEST_CASE("FileLoader", "[FileLoader]") {
    const auto file_path = data_dir / "FileLoader.json";
    const auto data = FileLoader::json(file_path);
    CHECK(data["id"].get<std::string>() == "6451902d4c88d542fa080f6b");
    CHECK(data["index"].get<unsigned>() == 25u);
}

TEST_CASE("file does not exist", "[FileLoader]") {
    const auto file_path = data_dir / "does_not_exist.json";
    CHECK_THROWS_AS(FileLoader::json(file_path), std::runtime_error);
}

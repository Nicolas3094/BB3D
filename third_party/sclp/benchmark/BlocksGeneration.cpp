#include <catch.hpp>

#include <filesystem>
#include <string>

#include "sclp/Core.hpp" // IWYU pragma: keep

using namespace sclp;

// ---------- parameters ----------

auto const data_dir = std::filesystem::path(__FILE__).parent_path() / "data/";

const auto file_name_list = std::vector<std::string>{
    "set_0_instance_1.json",
    "set_1_instance_1.json",
    "set_4_instance_1.json",
    "set_9_instance_1.json",
    "set_15_instance_1.json",
};

auto const MINIMUM_VOLUME_RATIO = double(0.98);

auto const MAXIMUM_NUMBER_OF_BLOCKS_TO_GENERATE = Index(10000);

// ---------- end of parameters ----------

struct Instance {
    std::string file_name;
    std::filesystem::path file_path;
    Json data;
    Input input;

    Instance(const std::string file_name)
        : file_name(file_name)
        , file_path(data_dir / file_name)
        , data(sclp::FileLoader::json(file_path))
        , input(sclp::InputReader::read(data)){};
};

TEST_CASE("GuillotineBlockGenerator - benchmark", "[benchmark][GuillotineBlockGenerator]") {
    auto const instance_list = std::vector<Instance>{file_name_list.begin(), file_name_list.end()};
    for (auto const & instance : instance_list) {
        auto generator = GuillotineBlockGenerator(instance.input.large_object(), MINIMUM_VOLUME_RATIO, MAXIMUM_NUMBER_OF_BLOCKS_TO_GENERATE);
        BENCHMARK(instance.file_name.c_str()) {
            auto blocks = generator.generate(instance.input.small_items());
            return blocks;
        };
    }
}

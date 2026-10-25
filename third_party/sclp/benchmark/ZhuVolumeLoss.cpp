#include <catch.hpp>

#include <filesystem>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types/Measure.hpp"

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
        auto volume_loss = ZhuVolumeLoss(instance.input.large_object());
        BENCHMARK(instance.file_name.c_str()) {
            volume_loss.update(instance.input.quantity_map(), instance.input.large_object().measurement());
            return volume_loss;
        };
    }
}

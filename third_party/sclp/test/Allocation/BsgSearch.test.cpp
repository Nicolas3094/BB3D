#include <catch.hpp>

#include <filesystem>
#include <memory>

#include "sclp/Allocation/BsgSearch.hpp"
#include "sclp/Allocation/Configuration.hpp"
#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/Constraint/AllConstraints.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Packing/Input.hpp"

using namespace sclp;

auto const INPUT_FILE_PATH = std::filesystem::path("test/data/Allocator/instance.json");

struct TestData {
    double alpha = 4.0;
    double beta = 1.0;
    double gamma = 0.2;
    double delta = 0.04;
    double block_minimum_volume_ratio = 0.98;
    Index maximum_number_of_blocks = 100;
    std::unique_ptr<Input> input;
    std::unique_ptr<GuillotineBlockGenerator> generator;
    std::vector<Block::Sptr> blocks;
    std::shared_ptr<ZhuVolumeLoss> volume_loss;
    std::unique_ptr<State> initial_state;
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<GreedySearch> greedy;
    double elite_threshold = 0.9;
    Index number_of_repetitions = 1;
    std::shared_ptr<FitnessComputer> fitness_computer;
    Seconds time_limit = 1000000; // bsg will stop because of the branching factor

    TestData(std::filesystem::path const & input_file_path) {
        // instance
        auto const data = FileLoader::json(input_file_path);
        input = std::make_unique<Input>(InputReader::read(data));
        generator = std::make_unique<GuillotineBlockGenerator>(
            input->large_object(),
            block_minimum_volume_ratio,
            maximum_number_of_blocks);
        blocks = generator->generate(input->small_items());
        volume_loss = std::make_shared<ZhuVolumeLoss>(input->large_object());
        initial_state = std::make_unique<State>(*input, blocks);
        configuration = std::make_shared<Configuration>(
            alpha,
            beta,
            gamma,
            delta);
        fitness_computer = std::make_shared<FitnessComputer>(
            *configuration,
            input->large_object(),
            volume_loss);
        greedy = std::make_shared<GreedySearch>(fitness_computer);
    }

    auto make_bsg(Index const & branching_factor) -> BsgSearch {
        return BsgSearch(
            branching_factor,
            time_limit,
            fitness_computer,
            greedy);
    }
};

TEST_CASE("increasing the branching factor increases improves the result", "[AllocatorBSG]") {
    auto data = TestData(INPUT_FILE_PATH);
    // low
    auto bsg_low = data.make_bsg(2);
    const auto state_low = bsg_low.search(*data.initial_state);
    REQUIRE_FALSE(state_low->valid());
    REQUIRE(AllConstraints::check(*data.input, state_low->packing));
    // high
    auto bsg_high = data.make_bsg(2);
    const auto state_high = bsg_high.search(*data.initial_state);
    REQUIRE_FALSE(state_high->valid());
    REQUIRE(AllConstraints::check(*data.input, state_high->packing));
    // compare
    CHECK(state_high->packing.allocated_volume() >= state_low->packing.allocated_volume());
}

TEST_CASE("compare bsg and greedy", "[AllocatorBSG]") {
    auto data = TestData(INPUT_FILE_PATH);
    // greedy
    auto greedy_result = data.greedy->search(*data.initial_state);
    REQUIRE(greedy_result.state.has_value());
    auto & greedy_final = greedy_result.state.value();
    greedy_final.packing.build();
    REQUIRE_FALSE(greedy_final.valid());
    REQUIRE(AllConstraints::check(*data.input, greedy_final.packing));
    // bsg
    auto bsg = data.make_bsg(2);
    auto const bsg_final = bsg.search(*data.initial_state);
    REQUIRE_FALSE(bsg_final->valid());
    REQUIRE(AllConstraints::check(*data.input, bsg_final->packing));
    // check
    //// final state must be invalid, that is, one cannot allocate anything else
    //// bsg must be better
    CHECK(bsg_final->packing.occupied_volume_ratio() > greedy_final.packing.occupied_volume_ratio());
    CHECK(bsg_final->packing.allocated_volume() > greedy_final.packing.allocated_volume());
}

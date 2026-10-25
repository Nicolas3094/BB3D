#include <catch.hpp>

#include <filesystem>

#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/Constraint/AllConstraints.hpp"

#include "sclp/Allocation/DoubleEffort.hpp"
#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
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
    std::shared_ptr<FitnessComputer> fitness_computer;
    Index initial_branching_factor = 2;
    Index maximum_branching_factor = 100000;
    Seconds time_limit = 1000000;

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

    auto make_double_effort(Index const & maximum_branching_factor, Seconds const & time_limit) -> DoubleEffort {
        return DoubleEffort(
            this->initial_branching_factor,
            maximum_branching_factor,
            time_limit,
            this->fitness_computer,
            this->greedy);
    }
};

TEST_CASE("stopping criterias", "[DoubleEffort]") {
    // if something is wrong with the stop criterias, then
    // the following tests will exceed the time limit
    // given to the tests
    auto data = TestData(INPUT_FILE_PATH);
    SECTION("limit by running time") {
        auto double_effort = data.make_double_effort(10000000, 0.5);
        const auto final_state = double_effort.search(*data.initial_state);
        CHECK_FALSE(final_state->valid());
        CHECK(AllConstraints::check(*data.input, final_state->packing));
        CHECK(final_state->packing.allocated_volume() > 0);
    }
    SECTION("limit by branching factor") {
        auto double_effort = data.make_double_effort(2, 100000000);
        const auto final_state = double_effort.search(*data.initial_state);
        CHECK_FALSE(final_state->valid());
        CHECK(AllConstraints::check(*data.input, final_state->packing));
        CHECK(final_state->packing.allocated_volume() > 0);
    }
}

TEST_CASE("increasing the branching factor increases improves the result", "[DoubleEffort]") {
    auto data = TestData(INPUT_FILE_PATH);
    // low branching factor
    auto double_effort_low = data.make_double_effort(2, 100000);
    const auto final_state_low = double_effort_low.search(*data.initial_state);
    REQUIRE_FALSE(final_state_low->valid());
    REQUIRE(AllConstraints::check(*data.input, final_state_low->packing));
    REQUIRE(final_state_low->packing.allocated_volume() > 0);
    // high branching factor
    auto double_effort_high = data.make_double_effort(3, 100000);
    const auto final_state_high = double_effort_high.search(*data.initial_state);
    REQUIRE_FALSE(final_state_high->valid());
    REQUIRE(AllConstraints::check(*data.input, final_state_high->packing));
    REQUIRE(final_state_high->packing.allocated_volume() > 0);
    // check
    CHECK(final_state_high->packing.allocated_volume() >= final_state_low->packing.allocated_volume());
}

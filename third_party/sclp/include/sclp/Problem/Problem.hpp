#ifndef SCLP_PROBLEM_PROBLEM_HPP_
#define SCLP_PROBLEM_PROBLEM_HPP_

#include <memory>

#include "sclp/Allocation/Configuration.hpp"
#include "sclp/Allocation/DoubleEffort.hpp"
#include "sclp/Allocation/FitnessComputer.hpp"
#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Allocation/GuillotineBlockGenerator.hpp"
#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/IO/FileLoader.hpp"
#include "sclp/IO/InputReader.hpp"
#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Packing/Input.hpp"
#include "sclp/Problem/CliParameters.hpp"

namespace sclp {

struct Problem {

    static constexpr int NUMBER_OF_ITEM_TYPES_THRESHOLD_FOR_BLOCK_MINIMUM_VOLUME_RATIO = 30;
    static constexpr double BLOCK_MINIMUM_VOLUME_RATIO_WEAKLY_HETEROGENEOUR = 1.00;
    static constexpr double BLOCK_MINIMUM_VOLUME_RATIO_STRONGLY_HETEROGENEOUR = 0.98;

    CliParameters cli_parameters;
    Timer timer;
    Json input_data;
    std::shared_ptr<Input> input;
    std::vector<Block::Sptr> blocks;
    std::shared_ptr<ZhuVolumeLoss> volume_loss;
    std::shared_ptr<State> initial_state;
    Configuration::Sptr configuration;
    Configuration::Sptr configuration_2;
    double configuration_switch_threshold;
    Seconds initialization_time;

    std::shared_ptr<FitnessComputer> fitness_computer;
    std::shared_ptr<GreedySearch> greedy;
    std::shared_ptr<DoubleEffort> double_effort;
    std::shared_ptr<State> best_state;

    Problem(const int argc, const char * const argv[])
        : cli_parameters(argc, argv) {
        return;
    }

    auto initialize() -> void {
        input_data = FileLoader::json(cli_parameters.io.instance_file);
        input = std::make_shared<Input>(InputReader::read(input_data));
        timer = Timer(cli_parameters.general.maximum_running_time_seconds);
        timer.resume();
        blocks = make_blocks(cli_parameters, *input);
        volume_loss = std::make_shared<ZhuVolumeLoss>(input->large_object());
        initial_state = std::make_shared<State>(*input, blocks);
        configuration = std::make_shared<Configuration>(
            cli_parameters.fitness_function.alpha,
            cli_parameters.fitness_function.beta,
            cli_parameters.fitness_function.gamma,
            cli_parameters.fitness_function.delta);
        configuration_2 = std::make_shared<Configuration>(
            cli_parameters.fitness_function.alpha_2,
            cli_parameters.fitness_function.beta_2,
            cli_parameters.fitness_function.gamma_2,
            cli_parameters.fitness_function.delta_2);
        configuration_switch_threshold = cli_parameters.fitness_function.configuration_switch_threshold;
        timer.stop();
        initialization_time = timer.elapsed();
        return;
    }

    auto solve() -> void {
        timer.resume();
        fitness_computer = std::make_shared<FitnessComputer>(
            *configuration,
            *configuration_2,
            configuration_switch_threshold,
            input->large_object(),
            volume_loss);
        greedy = std::make_shared<GreedySearch>(fitness_computer, cli_parameters.greedy.skip_type);
        double_effort = std::make_shared<DoubleEffort>(
            cli_parameters.tree_search.initial_branching_factor,
            cli_parameters.tree_search.maximum_branching_factor,
            timer.remaining(),
            fitness_computer,
            greedy);
        best_state = double_effort->search(*initial_state);
        timer.stop();
        return;
    }

    auto make_blocks(const CliParameters & cli_parameters, const Input & input) -> std::vector<Block::Sptr> {
        // select block_minimum_volume_ratio
        auto block_minimum_volume_ratio = cli_parameters.tree_search.block_minimum_volume_ratio;
        if (block_minimum_volume_ratio < 0.0) {
            auto const number_of_item_types = input.small_items().size();
            if (number_of_item_types < NUMBER_OF_ITEM_TYPES_THRESHOLD_FOR_BLOCK_MINIMUM_VOLUME_RATIO) {
                block_minimum_volume_ratio = BLOCK_MINIMUM_VOLUME_RATIO_WEAKLY_HETEROGENEOUR;
            } else {
                block_minimum_volume_ratio = BLOCK_MINIMUM_VOLUME_RATIO_STRONGLY_HETEROGENEOUR;
            }
        }
        // generate blocks
        auto block_generator = GuillotineBlockGenerator(
            input.large_object(),
            block_minimum_volume_ratio,
            Index(cli_parameters.tree_search.maximum_number_of_blocks));
        auto blocks = block_generator.generate(input.small_items());
        return blocks;
    }
};

} // namespace sclp

#endif

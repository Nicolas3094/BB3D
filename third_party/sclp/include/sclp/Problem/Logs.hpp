#ifndef SCLP_PROBLEM_LOGS_HPP_
#define SCLP_PROBLEM_LOGS_HPP_

#include <algorithm>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/DoubleEffort.hpp"
#include "sclp/Allocation/FitnessComputer.hpp"
#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Constraint/AllConstraints.hpp"
#include "sclp/Problem/Problem.hpp"

namespace sclp {

struct Logs {

    std::shared_ptr<Problem> problem;

    Logs(std::shared_ptr<Problem> const & problem)
        : problem(problem) {}

    auto run() -> void {
        if (problem->cli_parameters.io.irace) {
            irace(*problem->input, problem->best_state->packing, problem->timer.elapsed());
        } else {
            auto const & input = *problem->input;
            auto const & packing = problem->best_state->packing;
            auto output = Logs::make_output(input, packing);
            output["appendix"]["cli_parameters"] = problem->cli_parameters.to_json();
            output["appendix"]["input_statistics"] = Logs::make_input_statistics(input);
            output["appendix"]["packing_statistics"] = Logs::make_packing_statistics(input, packing);
            output["appendix"]["constraints_compliance"] = Logs::make_constraints_compliance(input, packing);
            output["appendix"]["initial_blocks"] = Logs::make_initial_blocks(problem->initial_state->blocks, input.large_object().measurement().prod());
            output["appendix"]["elapsed_times"] = Logs::make_elapsed_times(problem->fitness_computer, problem->greedy, problem->double_effort, problem->initialization_time, problem->timer.elapsed_wall());
            output["appendix"]["number_of_calls"] = Logs::make_number_of_calls(problem->fitness_computer, problem->greedy, problem->double_effort);
            output["appendix"]["solution_search"] = Logs::make_solution_search(problem->greedy);
            output["appendix"]["solution_history"] = Logs::make_solution_history(problem->double_effort);
            save(output, problem->cli_parameters.io.output_file);
        }
    }

    auto make_output(Input const & input, Packing const & packing) -> Json {
        auto obj = Json();
        // metadata
        obj["type"] = "output";
        obj["version"] = "0.5.0";
        // large object
        obj["large_object"] = Json();
        obj["large_object"]["measurement"] = Json();
        obj["large_object"]["measurement"]["x"] = input.large_object().measurement().x();
        obj["large_object"]["measurement"]["y"] = input.large_object().measurement().y();
        obj["large_object"]["measurement"]["z"] = input.large_object().measurement().z();
        // small items
        obj["small_items"] = Json::array();
        for (auto const & small_item : packing.allocated_small_items()) {
            auto small_item_obj = Json();
            small_item_obj["measurement"] = Json();
            small_item_obj["measurement"]["x"] = small_item.measurement().x();
            small_item_obj["measurement"]["y"] = small_item.measurement().y();
            small_item_obj["measurement"]["z"] = small_item.measurement().z();
            small_item_obj["position"] = Json();
            small_item_obj["position"]["x"] = small_item.initial_position().x();
            small_item_obj["position"]["y"] = small_item.initial_position().y();
            small_item_obj["position"]["z"] = small_item.initial_position().z();
            obj["small_items"].push_back(std::move(small_item_obj));
        }
        // appendix (empty)
        obj["appendix"] = Json();
        return obj;
    }

    auto make_input_statistics(Input const & input) -> Json {
        auto obj = Json();
        obj["large_object_volume"] = input.large_object().measurement().prod();
        obj["number_of_different_item_types"] = input.small_items().size();
        obj["total_quantity_of_small_items"] = input.total_quantity();
        obj["total_volume_of_small_items"] = input.total_volume();
        obj["volume_fraction_relative_to_large_object_volume"] = 100 * static_cast<double>(input.total_volume()) / input.large_object().measurement().prod();
        return obj;
    }

    auto make_packing_statistics(Input const & input, Packing const & packing) -> Json {
        auto obj = Json();
        obj["all_items_have_been_allocated_in_their_primary_orientation"] = std::all_of(packing.allocated_small_items().begin(), packing.allocated_small_items().end(), [](auto const & allocated_small_item) {
            return allocated_small_item.orientation() == OrientationType::XYZ;
        });
        obj["number_of_allocated_small_items"] = packing.allocated_small_items().size();
        obj["total_volume_of_allocated_small_items"] = packing.allocated_volume();
        obj["volume_fraction_relative_to_items_total_volume"] = 100 * static_cast<double>(packing.allocated_volume()) / input.total_volume();
        obj["volume_fraction_relative_to_large_object_volume"] = 100 * static_cast<double>(packing.allocated_volume()) / input.large_object().measurement().prod();
        return obj;
    }

    auto make_constraints_compliance(Input const & input, Packing const & packing) -> Json {
        auto obj = Json();
        obj["non_empty_packing_constraint"] = NonEmptyPackingConstraint::check(input, packing);
        obj["quantity_constraint"] = QuantityConstraint::check(packing).valid;
        obj["overlap_constraint"] = OverlapConstraint::check(packing).valid;
        obj["large_object_space_constraint"] = LargeObjectSpaceConstraint::check(input, packing).valid;
        obj["orientation_allocation_constraint"] = OrientationConstraint::check(packing).valid;
        obj["all_constraints"] = AllConstraints::check(input, packing);
        return obj;
    }

    template <typename T>
    struct Stats {
        T minimum;
        T maximum;
        double mean;
        double median;

        Stats(std::vector<T> & v) {
            std::stable_sort(v.begin(), v.end());
            minimum = v.front();
            maximum = v.back();
            mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
            median = v.size() % 2 == 0
                ? v[(v.size() / 2) - 1] + v[v.size() / 2] / 2.0
                : v[v.size() / 2];
        }

        auto to_json() -> Json {
            auto obj = Json();
            obj["minimum"] = minimum;
            obj["maximum"] = maximum;
            obj["mean"] = mean;
            obj["median"] = median;
            return obj;
        }
    };

    auto make_initial_blocks(std::vector<Block::Sptr> const & blocks, Volume const & large_object_volume) -> Json {
        auto obj = Json();
        obj["number"] = blocks.size();
        obj["large_object_volume"] = large_object_volume;
        // number of items
        auto number_of_items = std::vector<std::size_t>();
        for (auto const & block : blocks) {
            number_of_items.push_back(block->allocated_small_items().size());
        }
        obj["number_of_items"] = Stats(number_of_items).to_json();
        // volume
        auto volume = std::vector<Volume>();
        for (auto const & block : blocks) {
            volume.push_back(block->volume());
        }
        obj["volume"] = Stats(volume).to_json();
        // relative volume
        auto relative_volume = std::vector<double>();
        for (auto const & block : blocks) {
            relative_volume.push_back(static_cast<double>(block->volume()) / large_object_volume);
        }
        obj["relative_volume"] = Stats(relative_volume).to_json();
        // allocated volume
        auto allocated_volume = std::vector<Volume>();
        for (auto const & block : blocks) {
            allocated_volume.push_back(block->allocated_volume());
        }
        obj["allocated_volume"] = Stats(allocated_volume).to_json();
        // relative allocated volume
        auto relative_allocated_volume = std::vector<double>();
        for (auto const & block : blocks) {
            relative_allocated_volume.push_back(static_cast<double>(block->allocated_volume()) / large_object_volume);
        }
        obj["relative_allocated_volume"] = Stats(relative_allocated_volume).to_json();
        return obj;
    }

    auto make_elapsed_times(
        std::shared_ptr<FitnessComputer> const & fitness_computer,
        std::shared_ptr<GreedySearch> const & greedy,
        std::shared_ptr<DoubleEffort> const & double_effort,
        Seconds const & initialization_time,
        Seconds const & elapsed_wall_time)
        -> Json {
        auto obj = Json();
        obj["wall"] = elapsed_wall_time;
        obj["initialization"] = initialization_time;
        obj["allocation_whole"] = double_effort->timer.elapsed();
        obj["double_effort"] = double_effort->timer.elapsed();
        obj["greedy"] = greedy->timer.elapsed();
        obj["fitness_computer_partial"] = fitness_computer->partial_computation_timer.elapsed();
        obj["volume_loss_update"] = fitness_computer->volume_loss->update_timer.elapsed();
        return obj;
    }

    auto make_number_of_calls(
        std::shared_ptr<FitnessComputer> const & fitness_computer,
        std::shared_ptr<GreedySearch> const & greedy,
        std::shared_ptr<DoubleEffort> const & double_effort)
        -> Json {
        auto obj = Json();
        obj["double_effort"] = double_effort->number_of_calls;
        obj["bsg"] = double_effort->number_of_bsg_calls;
        obj["greedy"] = greedy->number_of_calls;
        obj["greedy_initial_skips"] = greedy->number_of_initial_skips;
        obj["greedy_of_partial_skips"] = greedy->number_of_partial_skips;
        obj["fitness_computer"] = fitness_computer->compute_number_of_calls;
        obj["volume_loss_update"] = fitness_computer->volume_loss->update_number_of_calls;
        return obj;
    }

    auto make_solution_search(std::shared_ptr<GreedySearch> const & greedy) -> Json {
        auto obj = Json();
        obj["minimum_depth"] = greedy->minimum_depth;
        obj["maximum_depth"] = greedy->maximum_depth;
        obj["average_depth"] = static_cast<double>(greedy->cumulative_depth) / greedy->number_of_calls;
        return obj;
    }

    auto make_solution_history(std::shared_ptr<DoubleEffort> const & double_effort) -> Json {
        auto obj = Json::array();
        for (auto const & entry : double_effort->solution_history.entries) {
            obj.push_back(entry);
        }
        return obj;
    }

    auto save(Json const & data, const std::string & output_file_path) -> void {
        auto file = std::ofstream(output_file_path, std::ifstream::out);
        if (!file.good()) {
            throw std::runtime_error("problem opening file for writing" + output_file_path);
        }
        file << data.dump(2, ' ', true);
        file << std::endl;
        file.close();
        return;
    }

    auto irace(Input const & input, Packing const & packing, double const & running_time_seconds) -> void {
        auto const large_object_volume = input.large_object().measurement().prod();
        auto const allocated_small_items_volume = packing.allocated_volume();
        auto relative_allocated_small_items_volume = static_cast<double>(allocated_small_items_volume) / large_object_volume;
        fmt::println("{} {}", -relative_allocated_small_items_volume, running_time_seconds);
        return;
    }
};

} // namespace sclp

#endif

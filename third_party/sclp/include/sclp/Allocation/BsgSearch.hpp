#ifndef SCLP_ALLOCATION_BSG_SEARCH_HPP_
#define SCLP_ALLOCATION_BSG_SEARCH_HPP_

#include <algorithm>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Allocation/SolutionHistory.hpp"
#include "sclp/Allocation/State.hpp"

namespace sclp {

class BsgSearch {
public:
    /**
     * Based on the work of:
     *
     * Araya, Ignacio and María Cristina Riff. “A beam search approach to the container loading problem.” Comput. Oper. Res. 43 (2014): 100-107.
     * https://doi.org/10.1016/j.cor.2013.09.003
     *
     */

    using StateUptr = std::unique_ptr<State>;
    using NextStatesMap = std::map<Volume, StateUptr, std::greater<Volume>>;

    struct HistoryEntry {
        Seconds time;
        Index branching_factor;
        Volume volume_usage;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(HistoryEntry, time, branching_factor, volume_usage);
    };

    Index branching_factor;
    Timer timer;
    FitnessComputer::Sptr fitness_computer;
    std::shared_ptr<GreedySearch> greedy;

    StateUptr best_state;
    SolutionHistory solution_history;

    BsgSearch(
        Index const & branching_factor,
        Seconds const & time_limit,
        FitnessComputer::Sptr const & fitness_computer,
        std::shared_ptr<GreedySearch> greedy)
        : branching_factor(branching_factor)
        , timer(time_limit)
        , fitness_computer(fitness_computer)
        , greedy(greedy)
        , best_state(nullptr) {}

    auto search(State const & root_state) -> StateUptr {
        // root_state: the state to be used as root for the tree search
        // current_states: the states in the current level of the search tree
        // incubent_states: the resultant states from expanding one state of the current states
        // next_states: the states in the next level of the search tree
        // final_states: the greedy expansion of the current_states
        this->timer.restart();
        this->timer.resume();
        this->best_state = std::make_unique<State>(root_state);
        std::vector<StateUptr> current_states;
        if (root_state.valid()) {
            current_states.push_back(std::make_unique<State>(root_state));
        }
        try {
            while (!current_states.empty()) {
                this->timer.check();
                NextStatesMap next_states;
                Index const number_of_expansions_per_state = this->branching_factor;
                for (auto & current_state : current_states) {
                    auto incubent_states = this->expand(current_state, number_of_expansions_per_state);
                    for (auto & incubent_state : incubent_states) {
                        update_next_states(incubent_state, next_states);
                    }
                }
                this->update_current_states(current_states, next_states);
            }
        } catch (TimeLimitException const & time_limit_exception) {}
        this->timer.stop();
        this->best_state->packing.build();
        this->timer.stop();
        return std::move(this->best_state);
    }

private:
    auto expand(StateUptr & state, Index const & number_of_expansions) -> std::vector<std::tuple<Volume, StateUptr>> {
        auto expansions = std::vector<std::tuple<Volume, StateUptr>>();
        this->fitness_computer->volume_loss->update(state->remaining_small_items_quantity, state->empty_spaces.maximum_measurement());
        while (state->valid()) { // loop required to remove useless empty spaces
            this->timer.check();
            // get best blocks - sort in decreasing order of fitness
            auto const partial_computation = this->fitness_computer->init(*state);
            auto best_blocks = select_best_blocks(*state, partial_computation, number_of_expansions);
            if (best_blocks.empty()) {
                // no block fits in the empty space selected, so remove it and restart
                state->empty_spaces.pop();
                continue;
            }
            for (auto const & block : best_blocks) {
                this->timer.check();
                auto expanded_state = std::make_unique<State>(*state);
                auto initial_position = this->fitness_computer->get_initial_position(partial_computation, block);
                expanded_state->add(block, initial_position);
                auto volume = this->estimate(*expanded_state);
                expansions.push_back({volume, std::move(expanded_state)});
            }
            break;
        }
        return expansions;
    }

    auto update_current_states(std::vector<StateUptr> & current_states, NextStatesMap & next_states) -> void {
        current_states.clear();
        auto i = Index(0);
        for (auto & element : next_states) {
            // be careful, const is being removed from elements of a multiset
            // since it is going to be cleared, it is not a problem
            current_states.push_back(std::move(const_cast<StateUptr &>(element.second)));
            ++i;
            if (i >= this->branching_factor) {
                break;
            }
        }
    }

    auto select_best_blocks(State const & state, FitnessComputer::PartialComputation const & partial_computation, Index const & count) -> std::vector<Block::Sptr> {
        auto values = std::vector<std::pair<double, Block::Sptr>>();
        for (auto const & block : state.blocks) {
            if (!this->fitness_computer->fit_in_space(partial_computation, block)) { continue; }
            values.push_back({this->fitness_computer->compute(partial_computation, block), block});
        }
        std::stable_sort(
            values.begin(),
            values.end(),
            [](std::pair<double, Block::Sptr> const & lhs, std::pair<double, Block::Sptr> const & rhs) {
                return lhs.first < rhs.first;
            });
        std::reverse(values.begin(), values.end());
        // transform into a vector
        auto selected_blocks = std::vector<Block::Sptr>{};
        for (auto i = Index(0); i < std::min(count, values.size()); ++i) {
            selected_blocks.push_back(values[i].second);
        }
        return selected_blocks;
    }

    auto update_next_states(
        std::tuple<Volume, StateUptr> & state_data,
        std::map<Volume, StateUptr, std::greater<Volume>> & storage)
        -> void {
        // make key using volume (for sorting) and hash (unicity)
        auto const key = std::get<Volume>(state_data);
        // try to find a state which yields a similar final state
        auto it = storage.find(key);
        if (it == storage.end()) {
            // key doesn't exist
            storage[key] = std::move(std::get<StateUptr>(state_data));
        }
    }

    auto estimate(State const & initial_state) -> Volume {
        auto best_volume = Volume(0);
        auto greedy_result = this->greedy->search(initial_state);
        if (greedy_result.state.has_value()) {
            auto & final_state = greedy_result.state.value();
            this->update_best_state(final_state);
        }
        best_volume = greedy_result.volume;
        return best_volume;
    }

    auto update_best_state(State const & state) -> void {
        if (state.packing.allocated_volume() > this->best_state->packing.allocated_volume()) {
            this->best_state = std::make_unique<State>(state);
            this->solution_history.add({this->timer.running_elapsed(),
                this->branching_factor,
                this->best_state->packing.allocated_volume()});
        }
        return;
    }
};

} // namespace sclp

#endif

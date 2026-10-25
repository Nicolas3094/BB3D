#ifndef SCLP_ALLOCATION_GREEDY_SEARCH_HPP_
#define SCLP_ALLOCATION_GREEDY_SEARCH_HPP_

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/FitnessComputer.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/EmptySpaceCollection/EmptySpaceCollection.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types.hpp"

template <>
struct boost::hash<sclp::Hash> {
    auto operator()(sclp::Hash const & hash) const noexcept -> std::size_t {
        return std::size_t(hash);
    }
};

namespace sclp {

class GreedySearch {
public:
    enum class SkipType {
        NONE,
        LOOSE,
        STRICT,
    };

    FitnessComputer::Sptr fitness_computer;
    SkipType skip_type;

    Index number_of_calls = 0;
    Index number_of_initial_skips = 0;
    Index number_of_partial_skips = 0;
    Timer timer;
    Index minimum_depth = std::numeric_limits<Index>::max();
    Index maximum_depth = 0;
    Index cumulative_depth = 0;
    boost::unordered::unordered_flat_map<Hash, Volume> search_history;

    GreedySearch(FitnessComputer::Sptr const & fitness_computer, SkipType const & skip_type = SkipType::NONE)
        : fitness_computer(fitness_computer)
        , skip_type(skip_type) {}

    struct GreedySearchResult {
        Volume volume;
        std::optional<State> state;
    };

    auto search(State const & initial_state) -> GreedySearchResult {
        ++this->number_of_calls;
        // check if state has already been seen
        auto search_history_it = this->search_history.end();
        if ((skip_type == SkipType::LOOSE) || (skip_type == SkipType::STRICT)) {
            search_history_it = this->search_history.find(initial_state.hash);
            if (search_history_it != this->search_history.end()) {
                ++this->number_of_initial_skips;
                return GreedySearchResult{search_history_it->second, std::nullopt};
            }
        }
        // initialize data for the search
        this->timer.resume();
        auto current_state = initial_state;
        auto intermediate_state_hashes = std::vector<Hash>();
        if ((skip_type == SkipType::LOOSE) || (skip_type == SkipType::STRICT)) {
            intermediate_state_hashes.push_back(current_state.hash);
        }
        while (current_state.valid() && (search_history_it == this->search_history.end())) {
            // find next block
            auto const partial_computation = this->fitness_computer->init(current_state);
            auto current_block = select_best_blocks(current_state.blocks, partial_computation);
            if (!current_block.has_value()) { // no block fits in the empty space selected, so remove it and restart
                current_state.empty_spaces.pop();
                continue;
            }
            // add block
            auto initial_position = this->fitness_computer->get_initial_position(partial_computation, *current_block);
            current_state.add(*current_block, initial_position);
            // check if state has been seen
            if (skip_type == SkipType::STRICT) {
                intermediate_state_hashes.push_back(current_state.hash);
                search_history_it = this->search_history.find(current_state.hash);
            }
        }
        this->timer.stop();
        // update debug info
        if (search_history_it == this->search_history.end()) {
            // the current state contains the result of greedy
            this->set_search_history(intermediate_state_hashes, current_state.packing.allocated_volume());
            auto const & solution_depth = current_state.packing.packings_to_allocate().size();
            this->minimum_depth = std::min(this->minimum_depth, solution_depth);
            this->maximum_depth = std::max(this->maximum_depth, solution_depth);
            this->cumulative_depth += solution_depth;
            return GreedySearchResult{current_state.packing.allocated_volume(), current_state};
        } else {
            // the search_history_it contains the result of greedy
            ++this->number_of_partial_skips;
            this->set_search_history(intermediate_state_hashes, search_history_it->second);
            return GreedySearchResult{search_history_it->second, std::nullopt};
        }
    }

private:
    auto select_best_blocks(
        std::vector<Block::Sptr> const & blocks,
        FitnessComputer::PartialComputation const & partial_computation)
        const -> std::optional<Block::Sptr> {
        auto best_fitness = Volume(0);
        auto best_block = std::optional<Block::Sptr>();
        for (auto const & block : blocks) {
            if (!this->fitness_computer->fit_in_space(partial_computation, block)) { continue; }
            auto fitness = this->fitness_computer->compute(partial_computation, block);
            if (fitness > best_fitness) {
                best_fitness = fitness;
                best_block = block;
            }
        }
        return best_block;
    }

    auto set_search_history(std::vector<Hash> const & intermediate_state_hashes, Volume const & allocated_volume) -> void {
        for (auto const & hash : intermediate_state_hashes) {
            this->search_history[hash] = allocated_volume;
        }
        return;
    }
};

} // namespace sclp

#endif

#ifndef SCLP_ALLOCATION_DOUBLE_EFFORT_HPP_
#define SCLP_ALLOCATION_DOUBLE_EFFORT_HPP_

#include "sclp/Types/Timer.hpp"
#define _USE_MATH_DEFINES

#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/BsgSearch.hpp"
#include "sclp/Allocation/GreedySearch.hpp"
#include "sclp/Allocation/SolutionHistory.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types.hpp"

namespace sclp {

class DoubleEffort {
public:
    /**
     * Based on the works of:
     *
     * Zhu, Wenbin et al. “The six elements to block-building approaches for the single container loading problem.” Applied Intelligence 37 (2012): 431-445.
     * https://doi.org/10.1007/s10489-012-0337-0
     *
     * Araya, Ignacio and María Cristina Riff. “A beam search approach to the container loading problem.” Comput. Oper. Res. 43 (2014): 100-107.
     * https://doi.org/10.1016/j.cor.2013.09.003
     *
     * Araya, Ignacio et al. “VCS: A new heuristic function for selecting boxes in the single container loading problem.” Comput. Oper. Res. 82 (2017): 27-35.
     * https://doi.org/10.1016/J.COR.2017.01.002
     *
     */

    using StateUptr = std::unique_ptr<State>;

    Index initial_branching_factor;
    Index maximum_branching_factor;
    Timer timer;
    FitnessComputer::Sptr fitness_computer;
    std::shared_ptr<GreedySearch> greedy;

    Index branching_factor;
    StateUptr best_state;

    // debug info
    Index number_of_calls = 0;
    Index number_of_bsg_calls = 0;
    SolutionHistory solution_history;

    DoubleEffort(
        Index const & initial_branching_factor,
        Index const & maximum_branching_factor,
        Seconds const & time_limit,
        FitnessComputer::Sptr const & fitness_computer,
        std::shared_ptr<GreedySearch> greedy)
        : initial_branching_factor(initial_branching_factor)
        , maximum_branching_factor(maximum_branching_factor)
        , timer(time_limit)
        , fitness_computer(fitness_computer)
        , greedy(greedy)
        , best_state(nullptr) {
        if (this->initial_branching_factor <= 1) {
            throw std::runtime_error(fmt::format(
                "initial_branching_factor of DoubleEffort must be greater or equal to 2, but it is {}",
                this->initial_branching_factor));
        }
        if (!(this->initial_branching_factor <= this->maximum_branching_factor)) {
            throw std::runtime_error(fmt::format(
                "initial_branching_factor ({}) of DoubleEffort must be smaller or equal to maximum_branching_factor ({})",
                this->initial_branching_factor,
                this->maximum_branching_factor));
        }
    }

    auto search(State const & root_state) -> StateUptr {
        this->timer.restart();
        this->timer.resume();
        ++this->number_of_calls;
        this->best_state = std::make_unique<State>(root_state);
        this->branching_factor = this->initial_branching_factor;
        try {
            while (this->branching_factor <= this->maximum_branching_factor) {
                this->timer.check();
                auto bsg = BsgSearch(
                    this->branching_factor,
                    this->timer.remaining(),
                    this->fitness_computer,
                    this->greedy);
                auto const bsg_initial_time = this->timer.running_elapsed();
                ++this->number_of_bsg_calls;
                auto best_state_found = bsg.search(root_state);
                this->update_best_state(std::move(best_state_found));
                this->solution_history.extend(bsg.solution_history, bsg_initial_time);
                this->branching_factor = std::round(M_SQRT2 * this->branching_factor);
            }
        } catch (TimeLimitException const & time_limit_exception) {}
        this->best_state->packing.build();
        this->timer.stop();
        return std::move(this->best_state);
    };

private:
    auto update_best_state(StateUptr && state) -> void {
        if (state->packing.allocated_volume() > this->best_state->packing.allocated_volume()) {
            this->best_state = std::move(state);
        }
        return;
    }
};

} // namespace sclp

#endif

#ifndef SCLP_ALLOCATION_FITNESS_COMPUTER_HPP_
#define SCLP_ALLOCATION_FITNESS_COMPUTER_HPP_

#include <cmath>
#include <memory>
#include <vector>

#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/Configuration.hpp"
#include "sclp/Allocation/Features.hpp"
#include "sclp/Allocation/State.hpp"
#include "sclp/Allocation/ZhuVolumeLoss.hpp"
#include "sclp/EmptySpaceCollection/EmptySpaceCollection.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class FitnessComputer {
public:
    /**
     * Based on the work of:
     *
     * Araya, Ignacio and María Cristina Riff. “A beam search approach to the container loading problem.” Comput. Oper. Res. 43 (2014): 100-107.
     * https://doi.org/10.1016/j.cor.2013.09.003
     *
     * Araya, Ignacio et al. “VCS: A new heuristic function for selecting boxes in the single container loading problem.” Comput. Oper. Res. 82 (2017): 27-35.
     * https://doi.org/10.1016/J.COR.2017.01.002
     *
     */

    using Sptr = std::shared_ptr<FitnessComputer>;

    Configuration configuration;
    Configuration configuration_2;
    double configuration_switch_threshold;

    LargeObject large_object;
    ZhuVolumeLoss::Sptr volume_loss;

    // debug info
    Index partial_computation_number_of_calls = 0;
    Timer partial_computation_timer;
    Index compute_number_of_calls = 0;

    FitnessComputer(
        Configuration const & configuration,
        Configuration const & configuration_2,
        double configuration_switch_threshold,
        LargeObject const & large_object,
        ZhuVolumeLoss::Sptr const & volume_loss)
        : configuration(configuration)
        , configuration_2(configuration_2)
        , configuration_switch_threshold(configuration_switch_threshold)
        , large_object(large_object)
        , volume_loss(volume_loss) {
        this->partial_computation_number_of_calls = 0;
        this->compute_number_of_calls = 0;
    }

    FitnessComputer(
        Configuration const & configuration,
        LargeObject const & large_object,
        ZhuVolumeLoss::Sptr const & volume_loss)
        // constructor which never uses the second configuration
        : FitnessComputer(
              configuration,
              Configuration(0.0, 0.0, 0.0, 0.0),
              100000.0,
              large_object,
              volume_loss) {}

    struct PartialComputation {
        std::array<std::vector<Area>, NUMBER_OF_FACES> broad_contact_areas;
        EmptySpace const & empty_space;
        Corner const & corner;
        QuantityMap const & remaining_small_items_quantity;
        double large_object_fill_rate;
    };

    auto init(State const & state) -> PartialComputation {
        this->partial_computation_timer.resume();
        ++this->partial_computation_number_of_calls;
        auto [empty_space, corner] = state.empty_spaces.top();
        auto large_object_fill_rate = static_cast<double>(state.packing.allocated_volume()) / large_object.measurement().prod();
        auto const & configuration = (large_object_fill_rate < configuration_switch_threshold)
            ? this->configuration
            : this->configuration_2;
        auto partial_computation = PartialComputation{
            configuration.pseudo_contact.find_areas(empty_space, state.packing),
            empty_space,
            corner,
            state.remaining_small_items_quantity,
            large_object_fill_rate};
        configuration.pseudo_contact.add_contact_areas_with_large_object(empty_space, large_object, partial_computation.broad_contact_areas);
        this->partial_computation_timer.stop();
        return partial_computation;
    }

    auto fit_in_space(PartialComputation const & partial_computation, Block::Sptr const & block) const -> bool {
        return block->measurement() <= partial_computation.empty_space.measurement();
    }

    auto compute_features(PartialComputation const & partial_computation, Block::Sptr const & block) -> Features {
        auto features = Features();
        auto const & configuration = (partial_computation.large_object_fill_rate < configuration_switch_threshold)
            ? this->configuration
            : this->configuration_2;
        auto initial_position = Anchor::position_at_corner(*block, partial_computation.empty_space, partial_computation.corner);
        auto const block_as_allocated = AllocatedRectangularCuboid(block->measurement(), initial_position);
        auto const contact_area = configuration.pseudo_contact.compute_contact_area(partial_computation.broad_contact_areas, block_as_allocated);
        auto const volume_loss = this->volume_loss->operator()(*block, partial_computation.empty_space);
        auto const relative_volume_loss = static_cast<double>(volume_loss) / partial_computation.empty_space.measurement().prod();
        features.V = block->allocated_volume();
        features.C = static_cast<double>(contact_area) / block->surface_area();
        features.L = 1 - relative_volume_loss;
        features.N = block->allocated_small_items().size();
        features.F = partial_computation.large_object_fill_rate;
        return features;
    }

    auto compute(PartialComputation const & partial_computation, Block::Sptr const & block) -> double {
        ++this->compute_number_of_calls;
        auto features = this->compute_features(partial_computation, block);
        auto const & configuration = (partial_computation.large_object_fill_rate < configuration_switch_threshold)
            ? this->configuration
            : this->configuration_2;
        auto fitness = features.V * std::pow(features.C, configuration.alpha) * std::pow(features.L, configuration.beta) * std::pow(features.N, -configuration.gamma);
        return fitness;
    }

    auto get_initial_position(PartialComputation const & partial_computation, Block::Sptr const & block) const -> DiscreteVector {
        return Anchor::position_at_corner(*block, partial_computation.empty_space, partial_computation.corner);
    }
};

} // namespace sclp

#endif

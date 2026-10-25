#ifndef SCLP_STATE_HPP_
#define SCLP_STATE_HPP_

#include <functional>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/EmptySpaceCollection/EmptySpaceCollection.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"
#include "sclp/Types.hpp"

namespace sclp {

class State {
public:
    Packing packing;
    EmptySpaceCollection empty_spaces;
    std::vector<Block::Sptr> blocks;
    QuantityMap remaining_small_items_quantity;
    Hash hash;

    State(
        Input const & input,
        std::vector<Block::Sptr> const & blocks)
        : packing()
        , empty_spaces(DiscreteVector::zeros())
        , blocks()
        , remaining_small_items_quantity()
        , hash() {
        this->_initialize_empty_spaces(input);
        this->blocks = blocks;
        this->_initialize_small_items_quantities(input);
        return;
    };

    State(State &&) = default;
    State(State const &) = default;
    State & operator=(State const &) = default;
    State & operator=(State &&) = default;

    auto valid() const noexcept -> bool {
        /**
         * a state is valid
         * when it can be expanded,
         * i.e. when there are empty spaces
         * and there are blocks
         */
        auto valid_spaces = !this->empty_spaces.empty();
        auto valid_blocks = !this->blocks.empty();
        return valid_spaces && valid_blocks;
    }

    auto add(Block::Sptr const & block, DiscreteVector const & initial_position) -> void {
        this->_add_block_and_cut_empty_spaces(block, initial_position);
        this->_update_remaining_small_items_quantity(block);
        this->_remove_invalid_blocks();
        this->_update_hash(block, initial_position);
        return;
    }

    static auto _has_valid_space(Block::Sptr const & block, DiscreteVector const & maximum_measurements, Volume const & maximum_volume) -> bool {
        auto valid_measurement = (block->measurement() <= maximum_measurements);
        auto valid_volume = (block->measurement().prod() <= maximum_volume);
        return valid_measurement && valid_volume;
    }

    auto _has_valid_quantities(Block const & block) const -> bool {
        for (auto const & [key, value] : block.quantity_map()) {
            auto it = this->remaining_small_items_quantity.find(key);
            if ((it == this->remaining_small_items_quantity.end()) && (value > 0)) {
                return false;
            } else if (it->second < value) {
                return false;
            }
        }
        return true;
    }

private:
    auto _add_block_and_cut_empty_spaces(Block::Sptr const & block, DiscreteVector const & initial_position) -> void {
        this->packing.add(block, initial_position);
        auto const space_for_cut = AllocatedRectangularCuboid(block->measurement(), initial_position);
        this->empty_spaces.cut(space_for_cut);
        return;
    }

    auto _update_remaining_small_items_quantity(Block::Sptr const & block) -> void {
        for (auto const & [key, value] : block->quantity_map()) {
            auto it = this->remaining_small_items_quantity.find(key);
            // check that quantity allocated is valid
            if ((it != this->remaining_small_items_quantity.end()) && (it->second < value)) {
                throw std::runtime_error("block has more items than available");
            }
            // reduce available value
            it->second -= value;
            // remove if it is zero
            if (it->second == 0) {
                this->remaining_small_items_quantity.erase(it);
            }
        }
        return;
    }

    auto _remove_invalid_blocks() -> void {
        auto const maximum_empty_space_measurements = this->empty_spaces.maximum_measurement();
        auto const maximum_empty_space_volume = this->empty_spaces.maximum_volume();
        std::erase_if(
            this->blocks,
            [this, &maximum_empty_space_measurements, &maximum_empty_space_volume](Block::Sptr const & block) -> bool { return !_has_valid_space(block, maximum_empty_space_measurements, maximum_empty_space_volume)
                                                                                                                            || !this->_has_valid_quantities(*block); });
        return;
    }

    auto _initialize_small_items_quantities(Input const & input) -> void {
        for (auto const & small_item : input.small_items()) {
            this->remaining_small_items_quantity[&small_item] = small_item.quantity();
        }
        return;
    }

    auto _initialize_empty_spaces(Input const & input) -> void {
        // construct
        this->empty_spaces = EmptySpaceCollection(input.large_object().measurement());
        // add space equivalent to large object
        this->empty_spaces.add(EmptySpace(
            input.large_object().measurement(),
            DiscreteVector::zeros()));
        // optimization parameters: minimum item measurements and volume
        auto minimum_item_measurement = DiscreteVector{
            std::numeric_limits<CoordinateType>::max(),
            std::numeric_limits<CoordinateType>::max(),
            std::numeric_limits<CoordinateType>::max()};
        auto minimum_item_volume = std::numeric_limits<Volume>::max();
        for (const auto & small_item : input.small_items()) {
            for (auto const orientation : small_item.orientation()) {
                auto const oriented_measurement = rotate(small_item.measurement(), orientation);
                minimum_item_measurement.x() = std::min(minimum_item_measurement.x(), oriented_measurement.x());
                minimum_item_measurement.y() = std::min(minimum_item_measurement.y(), oriented_measurement.y());
                minimum_item_measurement.z() = std::min(minimum_item_measurement.z(), oriented_measurement.z());
                minimum_item_volume = std::min(minimum_item_volume, small_item.measurement().prod());
            }
        }
        this->empty_spaces.set_minimum_item_measurement(minimum_item_measurement);
        this->empty_spaces.set_minimum_item_volume(minimum_item_volume);
        return;
    }

    auto _update_hash(Block::Sptr const & block, DiscreteVector const & initial_position) -> void {
        auto unique_hash = block->hash();
        auto const initial_position_hash = Hash(initial_position.x(), initial_position.y(), initial_position.z());
        unique_hash.combine_unique(initial_position_hash);
        this->hash.combine_commutative(unique_hash);
        return;
    }
};

} // namespace sclp

template <>
struct std::hash<sclp::State> {
    auto operator()(sclp::State const & state) const noexcept -> std::size_t {
        return state.hash;
    }
};

template <>
struct std::equal_to<sclp::State> {
    auto operator()(sclp::State const & lhs, sclp::State const & rhs) const noexcept -> bool {
        return lhs.hash == rhs.hash;
    }
};

#endif

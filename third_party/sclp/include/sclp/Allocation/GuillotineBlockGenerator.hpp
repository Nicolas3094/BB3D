#ifndef SCLP_ALLOCATION_GUILLOTINE_BLOCK_GENERATOR_HPP_
#define SCLP_ALLOCATION_GUILLOTINE_BLOCK_GENERATOR_HPP_

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep // IWYU pragma: keep

#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/LargeObject.hpp"

namespace sclp {

class GuillotineBlockGenerator {
private:
    static constexpr auto TOLERANCE = 1.0e-6;

    using HashSet = boost::unordered::unordered_flat_set<Hash>;

    struct SortByDecreasingOccupiedVolume {
        auto operator()(Block::Sptr const & lhs, Block::Sptr const & rhs) const {
            return std::tie(lhs->allocated_volume(), lhs->hash()) > std::tie(rhs->allocated_volume(), rhs->hash());
        }
    };

    using BlockSet = boost::container::flat_set<Block::Sptr, SortByDecreasingOccupiedVolume>;

public:
    GuillotineBlockGenerator(
        LargeObject const & large_object,
        double const & minimum_volume_ratio,
        Index const & maximum_number_of_blocks)
        : _large_object(large_object)
        , _minimum_volume_ratio(minimum_volume_ratio)
        , _maximum_number_of_blocks(maximum_number_of_blocks) {
        if ((this->minimum_volume_ratio() < 0.0) || (this->minimum_volume_ratio() > 1.0)) {
            throw std::runtime_error(fmt::format("minimum volume ratio must be a value between 0 and 1, but the value provided was {}", this->minimum_volume_ratio()));
        }
    }

    auto minimum_volume_ratio() const noexcept -> double { return this->_minimum_volume_ratio; }
    auto maximum_number_of_blocks() const noexcept -> Index { return this->_maximum_number_of_blocks; }
    auto large_object() const noexcept -> LargeObject const & { return this->_large_object; }

    auto generate(std::vector<SmallItem> const & small_items) const -> std::vector<Block::Sptr> {
        auto blocks = make_singular_blocks(small_items);
        combine_blocks(blocks);
        auto blocks_as_vector = std::vector<Block::Sptr>{blocks.begin(), blocks.end()};
        return blocks_as_vector;
    }

    auto make_singular_blocks(std::vector<SmallItem> const & small_items) const -> BlockSet {
        auto singular_blocks = BlockSet{};
        for (auto const & small_item : small_items) {
            for (auto const & orientation : small_item.orientation()) {
                auto block = std::make_shared<Block>();
                block->add(small_item, DiscreteVector::zeros(), orientation);
                block->update_hash();
                if (this->is_valid(*block)) {
                    singular_blocks.insert(std::move(block));
                }
            }
        }
        return singular_blocks;
    }

    auto combine_blocks(BlockSet & A) const -> void {
        auto P = BlockSet{A.begin(), A.end()};
        while (A.size() < this->_maximum_number_of_blocks) {
            auto N = generate_next_guillotine_blocks(A, P);
            if (N.empty()) {
                break;
            }
            for (auto & b : N) {
                A.insert(b);
            }
            P = std::move(N);
        }
        return;
    }

    auto join(
        Block::Sptr const & lhs,
        Block::Sptr const & rhs,
        Axis const & direction)
        const -> Block::Sptr {
        auto block = std::make_shared<Block>();
        block->add(lhs, DiscreteVector::zeros());
        block->add(rhs, DiscreteVector{direction == Axis::X ? lhs->measurement().x() : 0, direction == Axis::Y ? lhs->measurement().y() : 0, direction == Axis::Z ? lhs->measurement().z() : 0});
        block->build();
        return block;
    }

private:
    LargeObject _large_object;
    double _minimum_volume_ratio;
    Index _maximum_number_of_blocks;

    auto is_valid_join(
        Block::Sptr const & lhs,
        Block::Sptr const & rhs,
        Axis const & direction,
        BlockSet const & A,
        BlockSet const & N)
        const -> bool {
        auto block_join_measurement = DiscreteVector::zeros();
        switch (direction) {
        case Axis::X:
            block_join_measurement.x() = lhs->measurement().x() + rhs->measurement().x();
            block_join_measurement.y() = std::max(lhs->measurement().y(), rhs->measurement().y());
            block_join_measurement.z() = std::max(lhs->measurement().z(), rhs->measurement().z());
            break;
        case Axis::Y:
            block_join_measurement.x() = std::max(lhs->measurement().x(), rhs->measurement().x());
            block_join_measurement.y() = lhs->measurement().y() + rhs->measurement().y();
            block_join_measurement.z() = std::max(lhs->measurement().z(), rhs->measurement().z());
            break;
        case Axis::Z:
            block_join_measurement.x() = std::max(lhs->measurement().x(), rhs->measurement().x());
            block_join_measurement.y() = std::max(lhs->measurement().y(), rhs->measurement().y());
            block_join_measurement.z() = lhs->measurement().z() + rhs->measurement().z();
            break;
        default:
            throw std::runtime_error("direction not implemented");
        }
        // check if it fits into the large object
        if (!(block_join_measurement <= this->large_object().measurement())) {
            return false;
        }
        // check required minimum volume ratio
        auto const block_occupied_volume_ratio = (lhs->allocated_volume() + rhs->allocated_volume()) / static_cast<double>(block_join_measurement.prod());
        if (block_occupied_volume_ratio < _minimum_volume_ratio) {
            return false;
        }
        // check quantity
        auto quantity_map = lhs->quantity_map();
        for (auto const & [small_item, quantity] : rhs->quantity_map()) {
            quantity_map[small_item] += quantity;
        }
        for (auto const & [small_item, quantity] : quantity_map) {
            if (quantity > small_item->quantity()) {
                return false;
            }
        }
        // hash
        auto hash = Hash(
            block_join_measurement.x(),
            block_join_measurement.y(),
            block_join_measurement.z());
        for (auto const & [key, value] : quantity_map) {
            hash.combine_unique(key->index());
            hash.combine_unique(value);
        }
        // allocated volume
        auto b = std::make_shared<Block>();
        b->_hash = hash;
        b->_allocated_volume = lhs->allocated_volume() + rhs->allocated_volume();
        if (A.contains(b) || N.contains(b)) {
            return false;
        }
        return true;
    }

    auto generate_next_guillotine_blocks(BlockSet const & A, BlockSet const & P) const -> BlockSet {
        auto N = BlockSet{};
        for (auto const & block_left : P) {
            for (auto const & block_right : A) {
                for (auto const & direction : ALL_AXES) {
                    if (is_valid_join(block_left, block_right, direction, A, N)) {
                        N.insert(join(block_left, block_right, direction));
                    } else {
                        continue;
                    }
                    if (A.size() + N.size() >= this->_maximum_number_of_blocks) {
                        return N;
                    }
                }
            }
        }
        return N;
    }

    auto is_valid(Block const & block) const -> bool {
        bool has_something = !block.is_empty();
        bool valid_space = (block.measurement() <= this->large_object().measurement());
        bool valid_volume_ratio = (block.occupied_volume_ratio() > _minimum_volume_ratio - TOLERANCE);
        return has_something && block.valid_quantity() && valid_space && valid_volume_ratio;
    }
};

} // namespace sclp

#endif

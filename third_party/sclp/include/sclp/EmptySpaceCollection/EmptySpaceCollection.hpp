#ifndef SCLP_EMPTY_SPACE_COLLECTION_ZHU_HPP_
#define SCLP_EMPTY_SPACE_COLLECTION_ZHU_HPP_

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/EmptySpaceCollection/EmptySpaceCutter.hpp"
#include "sclp/Geometry/Anchor.hpp"
#include "sclp/Packing/EmptySpace.hpp"

namespace sclp {

class EmptySpaceCollection {
public:
    /**
     * Based on Zhu et al 2012:
     *
     * Zhu, Wenbin et al. “The six elements to block-building approaches for the single container loading problem.” Applied Intelligence 37 (2012): 431-445.
     *
     * https://doi.org/10.1007/s10489-012-0337-0
     */

    EmptySpaceCollection(DiscreteVector const & large_space_measurement)
        : minimum_item_measurement({0, 0, 0})
        , minimum_item_volume(0)
        , large_space(large_space_measurement, DiscreteVector::zeros()) {}

    auto set_minimum_item_measurement(const DiscreteVector & minimum_item_measurement) -> void {
        this->minimum_item_measurement = minimum_item_measurement;
        return;
    }

    auto set_minimum_item_volume(const Volume minimum_item_volume) -> void {
        this->minimum_item_volume = minimum_item_volume;
        return;
    }

    auto empty() const -> bool {
        return this->storage.empty();
    }

    auto size() const -> Index {
        return this->storage.size();
    }

    auto top() const -> std::pair<EmptySpace const &, Corner const &> {
        auto index = find_maximum_element_by_index();
        return {this->storage[index], std::get<Corner>(this->values[index])};
    }

    auto pop() -> void {
        auto index = find_maximum_element_by_index();
        swap(this->storage[index], this->storage.back());
        this->storage.pop_back();
        std::swap(this->values[index], this->values.back());
        this->values.pop_back();
        return;
    }

    auto add(EmptySpace && empty_space) -> void {
        // case 'empty_space' too small
        if ((empty_space.measurement().x() < this->minimum_item_measurement.x())
            || (empty_space.measurement().y() < this->minimum_item_measurement.y())
            || (empty_space.measurement().z() < this->minimum_item_measurement.z())
            || (empty_space.measurement().prod() < this->minimum_item_volume)) {
            return;
        }
        // case 'empty_space' inside one of the empty spaces in the collection
        for (const auto & other_empty_space : this->storage) {
            if (other_empty_space.contains(empty_space)) {
                return;
            }
        }
#ifdef SCLP_DEBUG
        if (std::find(this->storage.cbegin(), this->storage.cend(), empty_space) != this->storage.cend()) {
            throw std::logic_error("EmptySpaceCollection::add: empty_space already in collection");
        }
#endif
        this->storage.emplace_back(empty_space);
        auto [anchor_distance, anchor_corner] = Anchor::value_of(this->storage.back(), this->large_space);
        this->values.emplace_back(anchor_distance, this->storage.back().measurement().prod(), anchor_corner);
        return;
    }

    auto cut(const AllocatedRectangularCuboid & allocated_rectangular_cuboid) -> void {
        for (Index i = 0; i < this->storage.size();) {
            const auto & empty_space = this->storage[i];
            if (empty_space.overlaps(allocated_rectangular_cuboid)) {
                std::vector<EmptySpace> remaining_empty_spaces;
                EmptySpaceCutter::cut(
                    empty_space,
                    allocated_rectangular_cuboid,
                    remaining_empty_spaces);
                swap(this->storage[i], this->storage.back());
                std::swap(this->values[i], this->values.back());
                this->storage.pop_back();
                this->values.pop_back();
                for (auto it = remaining_empty_spaces.begin(); it != remaining_empty_spaces.end(); ++it) {
                    this->add(std::move(*it));
                }
            } else {
                ++i;
            }
        }
        return;
    }

    auto maximum_measurement() const noexcept -> DiscreteVector {
        auto maximum_measurement = DiscreteVector::zeros();
        for (auto const & empty_space : this->storage) {
            maximum_measurement.x() = std::max(maximum_measurement.x(), empty_space.measurement().x());
            maximum_measurement.y() = std::max(maximum_measurement.y(), empty_space.measurement().y());
            maximum_measurement.z() = std::max(maximum_measurement.z(), empty_space.measurement().z());
        }
        return maximum_measurement;
    }

    auto maximum_volume() const noexcept -> Volume {
        auto maximum_volume = Volume(0);
        for (auto const & empty_space : this->storage) {
            maximum_volume = std::max(maximum_volume, empty_space.measurement().prod());
        }
        return maximum_volume;
    }

private:
    using Value = std::tuple<Anchor::Distance, Volume, Corner>;

    std::vector<EmptySpace> storage;
    std::vector<Value> values;

    DiscreteVector minimum_item_measurement;
    Volume minimum_item_volume;

    AllocatedRectangularCuboid large_space;

    static auto swap(EmptySpace & lhs, EmptySpace & rhs) -> void {
        auto copy = lhs;
        lhs = rhs;
        rhs = copy;
        return;
    };

    auto find_maximum_element_by_index() const -> Index {
#ifdef SCLP_DEBUG
        if (this->storage.empty()) {
            throw std::runtime_error("storage is empty, cannot find maximum_element");
        }
        if (this->values.empty()) {
            throw std::runtime_error("values storage is empty, cannot find maximum_element");
        }
#endif
        auto maximum_value = std::make_tuple(std::numeric_limits<Anchor::Distance>::max(), Volume(0), Corner::NUMBER_OF_CORNERS);
        auto maximum_index = Index(0);
        for (auto i = Index(0); i < this->values.size(); ++i) {
            if (less(maximum_value, this->values[i])) {
                maximum_index = i;
                maximum_value = this->values[i];
            }
        }
        return maximum_index;
    }

    static auto less(Value const & lhs, Value const & rhs) -> bool {
        // the largest is the one with:
        //    1. smallest anchor distance
        //    2. tie break by largest volume
        //    3. tie break by corner number
        if (std::get<0>(lhs) > std::get<0>(rhs)) {
            return true;
        } else if (std::get<0>(lhs) < std::get<0>(rhs)) {
            return false;
        } else if (std::get<1>(lhs) < std::get<1>(rhs)) {
            return true;
        } else if (std::get<1>(lhs) > std::get<1>(rhs)) {
            return false;
        } else {
            return std::get<2>(lhs) < std::get<2>(rhs);
        }
    }
};

} // namespace sclp

#endif

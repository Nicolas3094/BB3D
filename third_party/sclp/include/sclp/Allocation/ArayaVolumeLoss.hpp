#ifndef SCLP_ALLOCATION_ARAYA_VOLUME_LOSS_HPP_
#define SCLP_ALLOCATION_ARAYA_VOLUME_LOSS_HPP_

#include <algorithm>
#include <optional>
#include <stdexcept>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"
#include "sclp/Packing/Block.hpp"

/**
 * This is based on the works:
 *
 * 1. Zhu, Wenbin et al. “The six elements to block-building approaches for the single container loading problem.” Applied Intelligence 37 (2012): 431-445.
 * 2. Araya, Ignacio and María Cristina Riff. “A beam search approach to the container loading problem.” Comput. Oper. Res. 43 (2014): 100-107.
 *
 * The code was inspired by the implementation of the repository:
 *
 * https://github.com/rilianx/Metasolver
 *
 * commit: f57af104c9d5012f6a32447081fa9d5dc458f24b
 *
 * This implementation has the goal of reproducing the results of Araya and Riff 2014,
 * so it was designed to reproduce the exact results of their code.
 *
 */

namespace sclp {

class ArayaVolumeLoss {
public:
    using Sptr = std::shared_ptr<ArayaVolumeLoss const>;
    using MeasurementType = DiscreteVector::scalar;
    using VolumeType = DiscreteVector::large_scalar;

    ArayaVolumeLoss(DiscreteVector const & maximal_measurements, QuantityMap const & remaining_small_items_quantity) {
        initialize_data(remaining_small_items_quantity, maximal_measurements.x(), Axis::X, maximum_measurement_reachable_with[Axis::X], small_items_to_reach_measurement[Axis::X]);
        initialize_data(remaining_small_items_quantity, maximal_measurements.y(), Axis::Y, maximum_measurement_reachable_with[Axis::Y], small_items_to_reach_measurement[Axis::Y]);
        initialize_data(remaining_small_items_quantity, maximal_measurements.z(), Axis::Z, maximum_measurement_reachable_with[Axis::Z], small_items_to_reach_measurement[Axis::Z]);
        return;
    }

    auto operator()(
        Block const & block,
        sclp::RectangularCuboid const & large_object,
        QuantityMap const & small_items_quantity)
        const -> VolumeType {
        /**
         * L: large object space measurements
         * E: effective large object space measurements
         * S: small object (block) space measurements
         * R: remaining space measurements
         * W: wasted space measurements
         * U: useful space measurements
         */
        auto const & Lx = large_object.measurement().x();
        auto const & Ly = large_object.measurement().y();
        auto const & Lz = large_object.measurement().z();
        auto const & Ex = maximum_measurement_reachable_with.at(Axis::X)[Lx];
        auto const & Ey = maximum_measurement_reachable_with.at(Axis::Y)[Ly];
        auto const & Ez = maximum_measurement_reachable_with.at(Axis::Z)[Lz];
        auto const & Sx = block.measurement().x();
        auto const & Sy = block.measurement().y();
        auto const & Sz = block.measurement().z();
        auto const Rx = Lx - Sx;
        auto const Ry = Ly - Sy;
        auto const Rz = Lz - Sz;
        auto const Wx = compute_measurement_waste(small_items_quantity, block, Rx, Axis::X);
        auto const Wy = compute_measurement_waste(small_items_quantity, block, Ry, Axis::Y);
        auto const Wz = compute_measurement_waste(small_items_quantity, block, Rz, Axis::Z);
        auto const Ux = Lx - Wx;
        auto const Uy = Ly - Wy;
        auto const Uz = Lz - Wz;
        auto const large_volume = VolumeType(Ex) * Ey * Ez;
        auto const small_volume = VolumeType(Ux) * Uy * Uz;
        auto const volume_loss = large_volume - small_volume;
        return volume_loss;
    }

    auto compute_measurement_waste(
        QuantityMap const & remaining_small_items_quantity,
        Block const & block,
        MeasurementType const & remaining_measurement,
        Axis const & axis)
        const -> MeasurementType {
        auto const & maximum_measurement_reachable_with = this->maximum_measurement_reachable_with.at(axis);
        auto const & small_items_to_reach_measurement = this->small_items_to_reach_measurement.at(axis);
        auto measurement_waste = remaining_measurement;
        auto maximum_measurement_reachable = maximum_measurement_reachable_with[remaining_measurement];
        while (maximum_measurement_reachable > 0) {
            for (auto it = small_items_to_reach_measurement[maximum_measurement_reachable].cbegin(); it != small_items_to_reach_measurement[maximum_measurement_reachable].cend(); ++it) {
                auto const & small_item_ptr = *it;
                auto const & number_of_small_items_in_block = block.quantity_map().contains(small_item_ptr)
                    ? block.quantity_map().at(small_item_ptr)
                    : Quantity(0);
                auto const & number_of_small_items_available = remaining_small_items_quantity.contains(small_item_ptr)
                    ? remaining_small_items_quantity.at(small_item_ptr)
                    : Quantity(0);
                if (number_of_small_items_available > number_of_small_items_in_block) {
                    measurement_waste = remaining_measurement - maximum_measurement_reachable;
                    maximum_measurement_reachable = 0;
                    break;
                }
            }
            if (maximum_measurement_reachable > 0) {
                maximum_measurement_reachable = maximum_measurement_reachable_with[maximum_measurement_reachable - 1];
            }
        }
        return measurement_waste;
    }

    static auto get_small_item_measurement(SmallItem const & small_item, Axis const & measurement_to_get, Axis const & axis) -> std::optional<MeasurementType> {
        /**
         * This method gets the measurement 'measurement_to_get' (X, Y, Z)
         * of 'small_item' if it is possible to orientate it so that
         * 'measurement_to_get' is in 'axis'
         *
         * For instance, if one has the small item with measurements
         * {3, 5, 7} and enabled orientations {XYZ, YXZ, ZYX} and
         * one wants to get:
         *
         * 1. the measurement Y in the axis X: then one will get 5,
         *    because the orientation YZX is enabled and in that
         *    orientation, the value of the measurement in the X
         *    axis is 5;
         * 2. the measurement Z in the axis Y: then one will get null,
         *    because there is no enabled orientation with the Z
         *    measurement in the Y axis;
         *
         * This method was designed to reproduce Araya and Riff 2014's
         * implementation. If it contains bugs, then those are also
         * present in the original implementation.
         *
         */
        switch (measurement_to_get) {
        case Axis::X:
            return small_item.measurement().get(axis);
        case Axis::Y:
            if (small_item.orientation()[OrientationType::YZX]) {
                return sclp::rotate(small_item.measurement(), OrientationType::YZX).get(axis);
            } else {
                return std::nullopt;
            }
        case Axis::Z:
            if (small_item.orientation()[OrientationType::ZXY]) {
                return sclp::rotate(small_item.measurement(), OrientationType::ZXY).get(axis);
            } else {
                return std::nullopt;
            }
        default:
            throw std::runtime_error("invalid Axis");
        }
    }

private:
    boost::unordered::unordered_flat_map<Axis, std::vector<MeasurementType>> maximum_measurement_reachable_with;
    boost::unordered::unordered_flat_map<Axis, std::vector<boost::unordered::unordered_flat_set<SmallItem const *>>> small_items_to_reach_measurement;

    auto initialize_data(
        QuantityMap const & small_items_quantity,
        MeasurementType const maximum_measurement,
        Axis const & axis,
        std::vector<MeasurementType> & maximum_measurement_reachable_with,
        std::vector<boost::unordered::unordered_flat_set<SmallItem const *>> & small_items_to_reach_measurement)
        -> void {
        // initialize
        auto measurement_reachable = std::vector<bool>(maximum_measurement + 1, false);
        maximum_measurement_reachable_with = std::vector<MeasurementType>(maximum_measurement + 1, MeasurementType(0));
        small_items_to_reach_measurement = std::vector<boost::unordered::unordered_flat_set<SmallItem const *>>(maximum_measurement + 1);
        measurement_reachable[0] = true;
        // populate
        for (auto it = small_items_quantity.cbegin(); it != small_items_quantity.cend(); ++it) {
            auto const & small_item_ptr = it->first;
            auto const & small_item_quantity = it->second;
            if (small_item_quantity == 0) { continue; }
            for (auto const & measurement_to_get : ALL_AXES) {
                auto optional_small_item_measurement = get_small_item_measurement(*small_item_ptr, measurement_to_get, axis);
                if (!optional_small_item_measurement.has_value()) { continue; }
                auto const & small_item_measurement = optional_small_item_measurement.value();
                // mark positions
                for (auto m = MeasurementType(0); m <= maximum_measurement - small_item_measurement; ++m) {
                    if (!measurement_reachable[m]) { continue; }
                    auto const maximum_usable_quantity = std::min(small_item_quantity, (maximum_measurement - m) / small_item_measurement);
                    for (auto q = maximum_usable_quantity; q >= 1; --q) {
                        auto const measurement = m + small_item_measurement * q;
                        measurement_reachable[measurement] = true;
                        small_items_to_reach_measurement[measurement].insert(small_item_ptr);
                    }
                }
            }
        }
        // populate the maximum measurement reachable with specific measurement
        maximum_measurement_reachable_with[0] = 0;
        for (auto m = MeasurementType(1); m <= maximum_measurement; ++m) {
            if (measurement_reachable[m]) {
                maximum_measurement_reachable_with[m] = m;
            } else {
                maximum_measurement_reachable_with[m] = maximum_measurement_reachable_with[m - 1];
            }
        }
        return;
    }
};

} // namespace sclp

#endif

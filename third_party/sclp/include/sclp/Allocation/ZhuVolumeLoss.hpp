#ifndef SCLP_ALLOCATION_ZHU_VOLUME_LOSS_HPP_
#define SCLP_ALLOCATION_ZHU_VOLUME_LOSS_HPP_

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <vector>

#include <boost/unordered/unordered_flat_map.hpp>

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Types/Axis.hpp"

/**
 * This is based on the works:
 *
 * 1. Zhu, Wenbin et al. “The six elements to block-building approaches for the single container loading problem.” Applied Intelligence 37 (2012): 431-445.
 * 2. Zhu, Wenbin and Andrew Lim. “A new iterative-doubling Greedy-Lookahead algorithm for the single container loading problem.” Eur. J. Oper. Res. 222 (2012): 408-417.
 *
 */

namespace sclp {

class ZhuVolumeLoss {
public:
    using Sptr = std::shared_ptr<ZhuVolumeLoss>;
    using MeasurementType = DiscreteVector::scalar;
    using VolumeType = DiscreteVector::large_scalar;

    Index update_number_of_calls;
    Timer update_timer;

    ZhuVolumeLoss(LargeObject const & large_object)
        : update_number_of_calls(0) {
        for (auto const & axis : ALL_AXES) {
            auto const size = large_object.measurement().get(axis) + 1;
            this->reachable[axis].resize(size, 0);
            this->temp_reachable[axis].resize(size, 0);
            this->maximum_reachable[axis].resize(size, 0);
        }
    }

    auto operator()(Block const & block, sclp::RectangularCuboid const & empty_space) const -> VolumeType {
        /**
         * B: block measurements
         * E: empty space measurements
         * R: remaining space measurements
         * U: useful space measurements
         * l: small space measurements
         */
        auto const & Bx = block.measurement().x();
        auto const & By = block.measurement().y();
        auto const & Bz = block.measurement().z();
        auto const & Ex = empty_space.measurement().x();
        auto const & Ey = empty_space.measurement().y();
        auto const & Ez = empty_space.measurement().z();
        auto const Rx = Ex - Bx;
        auto const Ry = Ey - By;
        auto const Rz = Ez - Bz;
#ifdef SCLP_DEBUG
        if (Rx >= static_cast<long int>(maximum_reachable.at(Axis::X).size())) {
            throw std::runtime_error(fmt::format("remaining space measurement x {} is larger than maximum measurement {}",
                Rx,
                maximum_reachable.at(Axis::X).size()));
        }
        if (Ry >= static_cast<long int>(maximum_reachable.at(Axis::Y).size())) {
            throw std::runtime_error(fmt::format("remaining space measurement y {} is larger than maximum measurement {}",
                Ry,
                maximum_reachable.at(Axis::X).size()));
        }
        if (Rz >= static_cast<long int>(maximum_reachable.at(Axis::X).size())) {
            throw std::runtime_error(fmt::format("remaining space measurement z {} is larger than maximum measurement {}",
                Rz,
                maximum_reachable.at(Axis::Z).size()));
        }
#endif
        auto const Ux = maximum_reachable.at(Axis::X)[Rx];
        auto const Uy = maximum_reachable.at(Axis::Y)[Ry];
        auto const Uz = maximum_reachable.at(Axis::Z)[Rz];
        auto const lx = Bx + Ux;
        auto const ly = By + Uy;
        auto const lz = Bz + Uz;
        auto const large_volume = VolumeType(Ex) * Ey * Ez;
        auto const small_volume = VolumeType(lx) * ly * lz;
        auto const volume_loss = large_volume - small_volume;
        return volume_loss;
    }

    auto update(QuantityMap const & items_quantity, DiscreteVector const & maximum_measurements) -> void {
        this->update_number_of_calls++;
        this->update_timer.resume();
        for (auto const & axis : ALL_AXES) {
            auto & reachable = this->reachable[axis];
            auto & temp_reachable = this->temp_reachable[axis];
            auto & maximum_reachable = this->maximum_reachable[axis];
            auto const maximum_measurement = maximum_measurements.get(axis);
#ifdef SCLP_DEBUG
            if (maximum_measurement > MeasurementType(maximum_reachable.size() - 1)) {
                throw std::runtime_error(fmt::format("given maximum measurement {} is larger than initial maximum measurement {}",
                    maximum_measurements.get(axis),
                    this->maximum_reachable[axis].size() - 1));
            }
#endif
            // find reachable positions
            std::fill(reachable.begin(), reachable.end(), 0);
            reachable[0] = 1;
            temp_reachable = reachable;
            for (auto const & [small_item, quantity] : items_quantity) {
                for (Quantity q = 0; q < quantity; ++q) {
                    for (auto const measurement_to_get : ALL_AXES) {
                        auto measurement_optional = get_small_item_measurement(*small_item, measurement_to_get, axis);
                        if (!measurement_optional) { continue; }
                        auto measurement = *measurement_optional;
                        for (int i = maximum_measurement - measurement; i >= 0; --i) {
                            if (reachable[i]) {
                                temp_reachable[i + measurement] = 1;
                            }
                        }
                    }
                    reachable = temp_reachable;
                }
            }
            // set maximum reachable
            maximum_reachable[0] = 0;
            for (MeasurementType m = 1; m <= maximum_measurement; ++m) {
                maximum_reachable[m] = reachable[m]
                    ? m
                    : maximum_reachable[m - 1];
            }
        }
        this->update_timer.stop();
        return;
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
         *    because the orientation YZX is enabled so one can get
         *    the measurement y in the X axis;
         * 2. the measurement Z in the axis Y: then one will get null,
         *    because there is no enabled orientation with the Z
         *    measurement in the Y axis;
         *
         */
        static constexpr auto lookup = std::array<std::array<std::pair<OrientationType, OrientationType>, NUMBER_OF_AXES>, NUMBER_OF_AXES>{{
            {{{OrientationType::XYZ, OrientationType::XZY}, {OrientationType::YXZ, OrientationType::ZXY}, {OrientationType::YZX, OrientationType::ZYX}}},
            {{{OrientationType::YXZ, OrientationType::YZX}, {OrientationType::XYZ, OrientationType::ZYX}, {OrientationType::XZY, OrientationType::ZXY}}},
            {{{OrientationType::ZXY, OrientationType::ZYX}, {OrientationType::XZY, OrientationType::YZX}, {OrientationType::XYZ, OrientationType::YXZ}}},
        }};

#ifdef SCLP_DEBUG
        if (static_cast<size_t>(measurement_to_get) >= NUMBER_OF_AXES || static_cast<size_t>(axis) >= NUMBER_OF_AXES) {
            throw std::runtime_error("invalid Axis");
        }
#endif

        auto const & [orient1, orient2] = lookup[static_cast<size_t>(measurement_to_get)][static_cast<size_t>(axis)];
        if (small_item.orientation()[orient1] || small_item.orientation()[orient2]) {
            switch (measurement_to_get) {
            case Axis::X:
                return small_item.measurement().x();
            case Axis::Y:
                return small_item.measurement().y();
            case Axis::Z:
                return small_item.measurement().z();
            default:
                throw std::runtime_error("invalid Axis");
            }
        }
        return std::nullopt;
    }

private:
    boost::unordered::unordered_flat_map<Axis, std::vector<uint8_t>> reachable;
    boost::unordered::unordered_flat_map<Axis, std::vector<uint8_t>> temp_reachable;
    boost::unordered::unordered_flat_map<Axis, std::vector<MeasurementType>> maximum_reachable;
};

} // namespace sclp

#endif

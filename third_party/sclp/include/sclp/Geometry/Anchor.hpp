#ifndef SCLP_GEOMETRY_ANCHORS_HPP_
#define SCLP_GEOMETRY_ANCHORS_HPP_

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <fmt/core.h>

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"

namespace sclp {

enum class Corner : std::size_t {
    /** Naming convention
     *
     * x (+front / -back), y (+left / -right), z(+top / -bottom)
     *
     *                top
     *           3 _________ 7
     *            /|       /|
     *           / |      / |
     *          /  |     /  |
     *       1 /________/ 5 |
     *        |    |    |   |
     *        |  2 |____|___| 6 left
     *        |   /     |   /
     *  back  |  /      |  / front
     *        | /       | /
     *        |/________|/ right
     *       0   bottom 4
     *
     * z
     * ^   y
     * |  /
     * |/
     * |----> x
     *
     */
    BACK_RIGHT_BOTTOM = 0b000,
    FRONT_RIGHT_BOTTOM = 0b100,
    BACK_LEFT_BOTTOM = 0b010,
    FRONT_LEFT_BOTTOM = 0b110,
    BACK_RIGHT_TOP = 0b001,
    FRONT_RIGHT_TOP = 0b101,
    BACK_LEFT_TOP = 0b011,
    FRONT_LEFT_TOP = 0b111,
    NUMBER_OF_CORNERS = 8,
};

struct Directions {
    bool x = false;
    bool y = false;
    bool z = false;

    auto to_corner() const noexcept -> Corner {
        auto corner_as_number = std::underlying_type_t<Corner>(0);
        // this is a hack that depends on the specific
        // implementation (values) of the corners
        corner_as_number += this->x << 2;
        corner_as_number += this->y << 1;
        corner_as_number += this->z << 0;
        return static_cast<Corner>(corner_as_number);
    }
};

class Anchor {
public:
    using Distance = DiscreteVector::scalar;

    Anchor() = delete;

    static auto value_of(AllocatedRectangularCuboid const & lhs, AllocatedRectangularCuboid const & rhs) -> std::pair<Distance, Corner> {
        /**
         * This method computes the Manhattan distance between the two
         * closest corners of the two given allocated rectangular cuboids.
         * When there are ties, it takes the corner with the minimum
         * value, in its numerical representation.
         *
         * This reproduces the behavior of Araya's algorithm.
         *
         * If one intends to change how the empty space is selected,
         * this is the one method to be changed.
         *
         */
        auto anchor_direction = Directions();
        auto anchor_distance = Distance(0);
        auto const initial_diff = (lhs.initial_position() - rhs.initial_position()).abs();
        auto const final_diff = (lhs.final_position() - rhs.final_position()).abs();
        // x
        if (initial_diff.x() <= final_diff.x()) {
            anchor_distance += initial_diff.x();
            anchor_direction.x = false;
        } else {
            anchor_distance += final_diff.x();
            anchor_direction.x = true;
        }
        // y
        if (initial_diff.y() <= final_diff.y()) {
            anchor_distance += initial_diff.y();
            anchor_direction.y = false;
        } else {
            anchor_distance += final_diff.y();
            anchor_direction.y = true;
        }
        // z
        if (initial_diff.z() <= final_diff.z()) {
            anchor_distance += initial_diff.z();
            anchor_direction.z = false;
        } else {
            anchor_distance += final_diff.z();
            anchor_direction.z = true;
        }
        auto anchor_corner = anchor_direction.to_corner();
        return {anchor_distance, anchor_corner};
    };

    static auto corner_position(AllocatedRectangularCuboid const & cuboid, Corner const & corner) -> DiscreteVector {
        switch (corner) {
        case Corner::BACK_RIGHT_BOTTOM:
            return DiscreteVector(cuboid.initial_position().x(), cuboid.initial_position().y(), cuboid.initial_position().z());
        case Corner::FRONT_RIGHT_BOTTOM:
            return DiscreteVector(cuboid.final_position().x(), cuboid.initial_position().y(), cuboid.initial_position().z());
        case Corner::BACK_LEFT_BOTTOM:
            return DiscreteVector(cuboid.initial_position().x(), cuboid.final_position().y(), cuboid.initial_position().z());
        case Corner::FRONT_LEFT_BOTTOM:
            return DiscreteVector(cuboid.final_position().x(), cuboid.final_position().y(), cuboid.initial_position().z());
        case Corner::BACK_RIGHT_TOP:
            return DiscreteVector(cuboid.initial_position().x(), cuboid.initial_position().y(), cuboid.final_position().z());
        case Corner::FRONT_RIGHT_TOP:
            return DiscreteVector(cuboid.final_position().x(), cuboid.initial_position().y(), cuboid.final_position().z());
        case Corner::BACK_LEFT_TOP:
            return DiscreteVector(cuboid.initial_position().x(), cuboid.final_position().y(), cuboid.final_position().z());
        case Corner::FRONT_LEFT_TOP:
            return DiscreteVector(cuboid.final_position().x(), cuboid.final_position().y(), cuboid.final_position().z());
        default:
            throw std::runtime_error(fmt::format("Corner position of corner value {} not implemented", static_cast<std::size_t>(corner)));
        }
    }

    static auto position_at_corner(
        RectangularCuboid const & small_object,
        AllocatedRectangularCuboid const & large_object,
        Corner const & corner)
        -> DiscreteVector {
        auto initial_position = large_object.initial_position();
#ifdef SCLP_DEBUG
        if (!(small_object.measurement() <= large_object.measurement())) {
            throw std::runtime_error(fmt::format(
                "Small object with measurements {} does not fit into large object with measurements {}",
                std::string(small_object.measurement()),
                std::string(large_object.measurement())));
        }
#endif
        // x
        switch (corner) {
        case Corner::FRONT_RIGHT_BOTTOM:
        case Corner::FRONT_LEFT_BOTTOM:
        case Corner::FRONT_RIGHT_TOP:
        case Corner::FRONT_LEFT_TOP:
            initial_position.x() += large_object.measurement().x() - small_object.measurement().x();
            break;
        default:
            break;
        }
        // y
        switch (corner) {
        case Corner::BACK_LEFT_BOTTOM:
        case Corner::FRONT_LEFT_BOTTOM:
        case Corner::BACK_LEFT_TOP:
        case Corner::FRONT_LEFT_TOP:
            initial_position.y() += large_object.measurement().y() - small_object.measurement().y();
            break;
        default:
            break;
        }
        // z
        switch (corner) {
        case Corner::BACK_RIGHT_TOP:
        case Corner::FRONT_RIGHT_TOP:
        case Corner::BACK_LEFT_TOP:
        case Corner::FRONT_LEFT_TOP:
            initial_position.z() += large_object.measurement().z() - small_object.measurement().z();
            break;
        default:
            break;
        }
        return initial_position;
    }
};

} // namespace sclp

#endif

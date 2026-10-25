#ifndef SCLP_ALLOCATION_PSEUDO_CONTACT_HPP_
#define SCLP_ALLOCATION_PSEUDO_CONTACT_HPP_

#include <array>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"
#include "sclp/Packing/Block.hpp"
#include "sclp/Packing/EmptySpace.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class PseudoContact {
public:
    PseudoContact(double const & relative_tolerance)
        : _relative_tolerance(relative_tolerance) {
        if (_relative_tolerance < 0.0) {
            throw std::runtime_error(fmt::format("relative tolerance must be non-negative, but value {} provided", relative_tolerance));
        }
        return;
    }

    inline auto relative_tolerance() const noexcept { return this->_relative_tolerance; }

    auto compute_contact_area(
        std::array<std::vector<Area>, NUMBER_OF_FACES> const & areas,
        sclp::AllocatedRectangularCuboid const & cuboid)
        const -> DiscreteVector::large_scalar {
        auto contact_area = DiscreteVector::large_scalar(0);
        for (auto const & face : ALL_FACES) {
            auto const absolute_tolerance = this->compute_absolute_tolerance(face, cuboid.measurement());
            auto const cuboid_area = cuboid.face(face);
            for (auto const & area : areas[static_cast<std::size_t>(face)]) {
                if (std::abs(cuboid_area.orthogonal_position() - area.orthogonal_position()) <= absolute_tolerance) {
                    auto overlapping_area = cuboid_area.overlaping_area_with(area);
                    if (overlapping_area.has_value()) {
                        contact_area += overlapping_area.value().prod();
                    }
                }
            }
        }
        return contact_area;
    }

    auto add_contact_areas_with_large_object(
        EmptySpace const & empty_space,
        RectangularCuboid const & large_object,
        std::array<std::vector<Area>, NUMBER_OF_FACES> & areas)
        const -> void {
        auto const x_absolute_tolerance = this->compute_absolute_tolerance(Face::BACK, empty_space.measurement());
        auto const y_absolute_tolerance = this->compute_absolute_tolerance(Face::RIGHT, empty_space.measurement());
        auto const z_absolute_tolerance = this->compute_absolute_tolerance(Face::BOTTOM, empty_space.measurement());
        if (empty_space.initial_position().x() - x_absolute_tolerance <= 0) {
            areas[static_cast<Index>(Face::BACK)].push_back(empty_space.face(Face::BACK));
        }
        if (large_object.measurement().x() <= empty_space.final_position().x() + x_absolute_tolerance) {
            areas[static_cast<Index>(Face::FRONT)].push_back(empty_space.face(Face::FRONT));
        }
        if (empty_space.initial_position().y() - y_absolute_tolerance <= 0) {
            areas[static_cast<Index>(Face::RIGHT)].push_back(empty_space.face(Face::RIGHT));
        }
        if (large_object.measurement().y() <= empty_space.final_position().y() + y_absolute_tolerance) {
            areas[static_cast<Index>(Face::LEFT)].push_back(empty_space.face(Face::LEFT));
        }
        if (empty_space.initial_position().z() - z_absolute_tolerance <= 0) {
            areas[static_cast<Index>(Face::BOTTOM)].push_back(empty_space.face(Face::BOTTOM));
        }
        if (large_object.measurement().z() <= empty_space.final_position().z() + z_absolute_tolerance) {
            areas[static_cast<Index>(Face::TOP)].push_back(empty_space.face(Face::TOP));
        }
        return;
    }

    auto find_areas(EmptySpace const & empty_space, Packing const & packing) const -> std::array<std::vector<Area>, NUMBER_OF_FACES> {
        /**
         * The areas may  overlap one another, in very specific cases:
         * when there area objects very small in the packing, the empty
         * space is large, and the relative tolerance is large, it may
         * happen that an object behind another one may satisfy the
         * conditions to be in pseudo-contact.
         *
         * This code does not handle that condition, presumibly because
         * it might never happen in real case scenarios.
         *
         * Handling that case would introduce 4 complications
         * to the current code.
         */
        auto areas = std::array<std::vector<Area>, NUMBER_OF_FACES>();
        for (auto const & face : ALL_FACES) {
            auto const opposite_face = compute_opposite_face(face);
            auto const absolute_tolerance = this->compute_absolute_tolerance(face, empty_space.measurement());
            auto const empty_space_area = empty_space.face(face);
            for (auto const & block_and_position : packing.packings_to_allocate()) {
                auto const block_area = compute_face_of(opposite_face, block_and_position);
                if (std::abs(empty_space_area.orthogonal_position() - block_area.orthogonal_position()) <= absolute_tolerance) {
                    // it should not be an absolute in the 'if',
                    // because the block area can only be in one direction: out.
                    // If there is no bug in positioning the items, this should
                    // not cause any issue, because one will use an empty space
                    // and there should be no block within it.
                    // However, that is not a robust implementation.
                    auto overlapping_area = block_area.overlaping_area_with(empty_space_area);
                    if (overlapping_area.has_value()) {
                        areas[static_cast<std::size_t>(face)].push_back(std::move(overlapping_area.value()));
                    }
                }
            }
        }
        return areas;
    }

    static auto compute_opposite_face(Face const & face) -> Face {
        switch (face) {
        case Face::BACK:
            return Face::FRONT;
        case Face::FRONT:
            return Face::BACK;
        case Face::RIGHT:
            return Face::LEFT;
        case Face::LEFT:
            return Face::RIGHT;
        case Face::BOTTOM:
            return Face::TOP;
        case Face::TOP:
            return Face::BOTTOM;
        }
        throw std::runtime_error(fmt::format("opposite_face not implemented for face {}", static_cast<int>(face)));
    }

    auto compute_absolute_tolerance(Face const & face, DiscreteVector const & measurement) const noexcept -> DiscreteVector::scalar {
        double tolerance = this->relative_tolerance();
        switch (face) {
        case Face::BACK:
        case Face::FRONT:
            tolerance *= measurement.x();
            break;
        case Face::RIGHT:
        case Face::LEFT:
            tolerance *= measurement.y();
            break;
        case Face::BOTTOM:
        case Face::TOP:
            tolerance *= measurement.z();
            break;
        }
        tolerance = std::floor(tolerance);
        return static_cast<DiscreteVector::scalar>(tolerance);
    }

    static auto compute_face_of(Face const & face, std::tuple<Block::Sptr, DiscreteVector> const & block_and_position) -> Area {
        auto const & block = std::get<0>(block_and_position);
        auto const & initial_position = std::get<1>(block_and_position);
        auto const & measurement = block->measurement();
        auto const final_position = initial_position + measurement;
        auto plane = Plane::XY;
        auto orthogonal_position = DiscreteVector2D::scalar(0);
        switch (face) {
        case Face::BACK:
            plane = Plane::YZ;
            orthogonal_position = initial_position.x();
            break;
        case Face::FRONT:
            plane = Plane::YZ;
            orthogonal_position = final_position.x();
            break;
        case Face::RIGHT:
            plane = Plane::XZ;
            orthogonal_position = initial_position.y();
            break;
        case Face::LEFT:
            plane = Plane::XZ;
            orthogonal_position = final_position.y();
            break;
        case Face::BOTTOM:
            plane = Plane::XY;
            orthogonal_position = initial_position.z();
            break;
        case Face::TOP:
            plane = Plane::XY;
            orthogonal_position = final_position.z();
            break;
        }
        auto area = Area::from_measurement(plane, project(plane, measurement), project(plane, initial_position), orthogonal_position);
        return area;
    }

private:
    double _relative_tolerance;
};

} // namespace sclp

#endif

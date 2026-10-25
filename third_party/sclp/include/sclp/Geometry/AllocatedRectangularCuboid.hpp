#ifndef SCLP_GEOMETRY_ALLOCATED_RECTANGULAR_CUBOID_HPP_
#define SCLP_GEOMETRY_ALLOCATED_RECTANGULAR_CUBOID_HPP_

#include <ostream>
#include <string>

#include <fmt/core.h>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/AllocatedObject.hpp"
#include "sclp/Geometry/Area.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"

namespace sclp {

class AllocatedRectangularCuboid : public RectangularCuboid, public AllocatedObject {
public:
    AllocatedRectangularCuboid(DiscreteVector const & measurement, DiscreteVector const & initial_position) noexcept
        : RectangularCuboid(measurement)
        , AllocatedObject(initial_position, initial_position + measurement) {}

    auto operator==(AllocatedRectangularCuboid const & that) const noexcept -> bool {
        return RectangularCuboid::operator==(that) && AllocatedObject::operator==(that);
    }

    friend auto to_json(Json & obj, AllocatedRectangularCuboid const & allocated_rectangular_cuboid) -> void {
        obj["initial_position"] = allocated_rectangular_cuboid.initial_position();
        obj["measurement"] = allocated_rectangular_cuboid.measurement();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, AllocatedRectangularCuboid const & allocated_rectangular_cuboid) -> std::ostream & {
        return os << std::string(allocated_rectangular_cuboid);
    }

    auto contains(AllocatedRectangularCuboid const & that) const -> bool;
    auto overlaps(AllocatedRectangularCuboid const & that) const -> bool;
    auto touches(AllocatedRectangularCuboid const & that) const -> bool;
    auto overlaps_or_touches(AllocatedRectangularCuboid const & that) const -> bool;
    auto on(AllocatedRectangularCuboid const & that) const noexcept -> bool;

    auto projection(Plane const & plane) const noexcept -> Area {
        return Area::from_measurement(plane, project(plane, this->measurement()), project(plane, this->initial_position()), 0);
    }

    auto face(Face const & face) const noexcept -> Area {
        auto plane = Plane::XY;
        auto orthogonal_position = DiscreteVector2D::scalar(0);
        switch (face) {
        case Face::BACK:
            plane = Plane::YZ;
            orthogonal_position = this->initial_position().x();
            break;
        case Face::FRONT:
            plane = Plane::YZ;
            orthogonal_position = this->final_position().x();
            break;
        case Face::RIGHT:
            plane = Plane::XZ;
            orthogonal_position = this->initial_position().y();
            break;
        case Face::LEFT:
            plane = Plane::XZ;
            orthogonal_position = this->final_position().y();
            break;
        case Face::BOTTOM:
            plane = Plane::XY;
            orthogonal_position = this->initial_position().z();
            break;
        case Face::TOP:
            plane = Plane::XY;
            orthogonal_position = this->final_position().z();
            break;
        }
        auto area = Area::from_measurement(plane, project(plane, this->measurement()), project(plane, this->initial_position()), orthogonal_position);
        return area;
    }
};

} // namespace sclp

#endif

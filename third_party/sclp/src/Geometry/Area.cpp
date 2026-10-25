#include "sclp/Geometry/Area.hpp"

#include <cassert>
#include <stdexcept>

#include <fmt/core.h>

namespace sclp {

auto project(Plane const & plane, DiscreteVector const & v) -> DiscreteVector2D {
    assert(plane == Plane::XY || plane == Plane::XZ || plane == Plane::YZ);
    switch (plane) {
    case Plane::XY:
        return DiscreteVector2D{v.x(), v.y()};
    case Plane::XZ:
        return DiscreteVector2D{v.x(), v.z()};
    case Plane::YZ:
        return DiscreteVector2D{v.y(), v.z()};
    default:
        throw std::runtime_error(fmt::format("projection in plane {} not implemented", static_cast<int>(plane)));
    }
}

} // namespace sclp

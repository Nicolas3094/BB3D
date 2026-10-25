#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"

namespace sclp {

auto AllocatedRectangularCuboid::contains(AllocatedRectangularCuboid const & that) const -> bool {
    return (this->initial_position() <= that.initial_position()) && (that.final_position() <= this->final_position());
}

auto AllocatedRectangularCuboid::overlaps(AllocatedRectangularCuboid const & that) const -> bool {
    return (this->initial_position() < that.final_position()) && (that.initial_position() < this->final_position());
}

auto AllocatedRectangularCuboid::touches(AllocatedRectangularCuboid const & that) const -> bool {
    return !this->overlaps(that) && (this->initial_position() <= that.final_position()) && (that.initial_position() <= this->final_position());
}

auto AllocatedRectangularCuboid::overlaps_or_touches(AllocatedRectangularCuboid const & that) const -> bool {
    return (this->initial_position() <= that.final_position()) && (that.initial_position() <= this->final_position());
}

auto AllocatedRectangularCuboid::on(AllocatedRectangularCuboid const & that) const noexcept -> bool {
    // bottom of this matches top of that
    // AND there is intersection in x
    // AND there is intersection in y
    return (this->initial_position().z() == that.final_position().z())
        && (this->projection(Plane::XY).overlaps(that.projection(Plane::XY)));
}

} // namespace sclp

#ifndef SCLP_GEOMETRY_RECTANGULAR_CUBOID_HPP_
#define SCLP_GEOMETRY_RECTANGULAR_CUBOID_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/DiscreteVector.hpp"

namespace sclp {

class RectangularCuboid {
public:
    RectangularCuboid(DiscreteVector const & measurement) noexcept
        : _measurement(measurement) {}

    auto measurement() const noexcept -> DiscreteVector const & { return _measurement; }

    auto surface_area() const noexcept -> DiscreteVector::large_scalar {
        auto xy = this->measurement().x() * this->measurement().y();
        auto xz = this->measurement().x() * this->measurement().z();
        auto yz = this->measurement().y() * this->measurement().z();
        return 2 * (xy + xz + yz);
    }

    auto operator==(RectangularCuboid const & that) const noexcept -> bool {
        return this->measurement() == that.measurement();
    }

    friend auto to_json(Json & obj, RectangularCuboid const & rectangular_cuboid) -> void {
        obj["measurement"] = rectangular_cuboid.measurement();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, RectangularCuboid const & rectangular_cuboid) -> std::ostream & {
        return os << std::string(rectangular_cuboid);
    }

protected:
    DiscreteVector _measurement;
};

} // namespace sclp

#endif

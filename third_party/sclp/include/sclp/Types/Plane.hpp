#ifndef SCLP_TYPES_PLANE_HPP_
#define SCLP_TYPES_PLANE_HPP_

#include <array>

namespace sclp {

enum class Plane {
    XY = 0,
    XZ = 1,
    YZ = 2
};

static constexpr auto ALL_PLANES = std::array{Plane::XY, Plane::XZ, Plane::YZ};

static constexpr auto NUMBER_OF_PLANES = ALL_PLANES.size();

static_assert(NUMBER_OF_PLANES == static_cast<int>(Plane::YZ) + 1, "the number of planes must be 1 greater than the last value available");

} // namespace sclp

#endif

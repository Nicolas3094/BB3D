#ifndef SCLP_GEOMETRY_DISCRETE_VECTOR_HPP_
#define SCLP_GEOMETRY_DISCRETE_VECTOR_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Vector.hpp"
#include "sclp/Geometry/Vector2D.hpp"

namespace sclp {

using DiscreteVector = details::Vector<CoordinateType, LargeCoordinateType>;
using DiscreteVector2D = details::Vector2D<CoordinateType, LargeCoordinateType>;

} // namespace sclp

#endif

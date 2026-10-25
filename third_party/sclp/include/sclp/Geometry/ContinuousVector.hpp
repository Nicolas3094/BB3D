#ifndef SCLP_GEOMETRY_CONTINUOUS_VECTOR_HPP_
#define SCLP_GEOMETRY_CONTINUOUS_VECTOR_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Vector.hpp"

namespace sclp {

using ContinuousVector = details::Vector<DistanceType>;

} // namespace sclp

#endif

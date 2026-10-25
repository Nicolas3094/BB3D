#ifndef SCLP_TYPES_MEASURE_HPP_
#define SCLP_TYPES_MEASURE_HPP_

#include <cstddef>
#include <cstdint>

namespace sclp {

using Index = std::size_t;

using Quantity = std::int_fast16_t;

using CoordinateType = std::int_fast16_t;

using LargeCoordinateType = std::int_fast64_t;

using DistanceType = double;

using Volume = LargeCoordinateType;

} // namespace sclp

#endif

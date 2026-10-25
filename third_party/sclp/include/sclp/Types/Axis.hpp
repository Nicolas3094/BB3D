#ifndef SCLP_TYPES_AXIS_HPP_
#define SCLP_TYPES_AXIS_HPP_

#include <array>

namespace sclp {

enum class Axis {
    X = 0,
    Y = 1,
    Z = 2
};

static constexpr auto ALL_AXES = std::array{Axis::X, Axis::Y, Axis::Z};

static constexpr auto NUMBER_OF_AXES = ALL_AXES.size();

static_assert(NUMBER_OF_AXES == static_cast<int>(Axis::Z) + 1, "the number of axes must be 1 greater than the last value available");

} // namespace sclp

#endif

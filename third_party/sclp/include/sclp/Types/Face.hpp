#ifndef SCLP_TYPES_FACE_HPP_
#define SCLP_TYPES_FACE_HPP_

#include <array>

// IWYU pragma: end_exports

namespace sclp {

enum class Face {
    BACK = 0,
    FRONT = 1,
    RIGHT = 2,
    LEFT = 3,
    BOTTOM = 4,
    TOP = 5,
};

static constexpr auto ALL_FACES = std::array{
    Face::BACK,
    Face::FRONT,
    Face::RIGHT,
    Face::LEFT,
    Face::BOTTOM,
    Face::TOP,
};

static constexpr auto NUMBER_OF_FACES = ALL_FACES.size();

static_assert(NUMBER_OF_FACES == static_cast<int>(Face::TOP) + 1, "the number of faces must be 1 greater than the last value available");

} // namespace sclp

#endif

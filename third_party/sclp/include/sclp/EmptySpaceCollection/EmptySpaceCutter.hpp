#ifndef SCLP_EMPTY_SPACE_CUTTER_HPP_
#define SCLP_EMPTY_SPACE_CUTTER_HPP_

#include <vector>

#include "sclp/Packing/EmptySpace.hpp"

namespace sclp {

class EmptySpaceCutter {
public:
    EmptySpaceCutter() = delete;
    ~EmptySpaceCutter() = delete;
    EmptySpaceCutter(const EmptySpaceCutter &) = delete;
    EmptySpaceCutter(EmptySpaceCutter &&) = delete;
    EmptySpaceCutter & operator=(const EmptySpaceCutter &) = delete;
    EmptySpaceCutter & operator=(EmptySpaceCutter &&) = delete;

    static auto cut(
        const EmptySpace & empty_space,
        const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid,
        std::vector<EmptySpace> & container) -> void;
};

} // namespace sclp

#endif

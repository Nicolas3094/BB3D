#include "sclp/EmptySpaceCollection/EmptySpaceCutter.hpp"

namespace sclp::details {

// ========== Auxiliary Methods - Declarations ==========

auto cut_space_back(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

auto cut_space_front(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

auto cut_space_right(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

auto cut_space_left(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

auto cut_space_bottom(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

auto cut_space_top(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace;

// ========== Auxiliary Methods - Definitions ==========

auto cut_space_back(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        allocated_rectangular_cuboid.initial_position().x() - empty_space.initial_position().x(),
        empty_space.measurement().y(),
        empty_space.measurement().z()};
    auto initial_position = DiscreteVector{
        empty_space.initial_position().x(),
        empty_space.initial_position().y(),
        empty_space.initial_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

auto cut_space_front(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        empty_space.final_position().x() - allocated_rectangular_cuboid.final_position().x(),
        empty_space.measurement().y(),
        empty_space.measurement().z()};
    auto initial_position = DiscreteVector{
        allocated_rectangular_cuboid.final_position().x(),
        empty_space.initial_position().y(),
        empty_space.initial_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

auto cut_space_right(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        empty_space.measurement().x(),
        allocated_rectangular_cuboid.initial_position().y() - empty_space.initial_position().y(),
        empty_space.measurement().z()};
    auto initial_position = DiscreteVector{
        empty_space.initial_position().x(),
        empty_space.initial_position().y(),
        empty_space.initial_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

auto cut_space_left(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        empty_space.measurement().x(),
        empty_space.final_position().y() - allocated_rectangular_cuboid.final_position().y(),
        empty_space.measurement().z()};
    auto initial_position = DiscreteVector{
        empty_space.initial_position().x(),
        allocated_rectangular_cuboid.final_position().y(),
        empty_space.initial_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

auto cut_space_bottom(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        empty_space.measurement().x(),
        empty_space.measurement().y(),
        allocated_rectangular_cuboid.initial_position().z() - empty_space.initial_position().z()};
    auto initial_position = DiscreteVector{
        empty_space.initial_position().x(),
        empty_space.initial_position().y(),
        empty_space.initial_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

auto cut_space_top(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid)
    -> EmptySpace {
    auto measurement = DiscreteVector{
        empty_space.measurement().x(),
        empty_space.measurement().y(),
        empty_space.final_position().z() - allocated_rectangular_cuboid.final_position().z()};
    auto initial_position = DiscreteVector{
        empty_space.initial_position().x(),
        empty_space.initial_position().y(),
        allocated_rectangular_cuboid.final_position().z()};
    return EmptySpace{std::move(measurement), std::move(initial_position)};
}

} // namespace sclp::details

namespace sclp {

auto EmptySpaceCutter::cut(
    const EmptySpace & empty_space,
    const sclp::AllocatedRectangularCuboid & allocated_rectangular_cuboid,
    std::vector<EmptySpace> & container) -> void {
    if (!empty_space.overlaps(allocated_rectangular_cuboid)) {
        // Nothing should be done in this condition.
        // Ideally, this condition should never be true.
        return;
    }
    /* TODO: the 'if' above is not really necessary if one
     * implements a collection of empty spaces that can
     * handle Empty Spaces which are inside one another.
     * In the case of the 'if' above, three Empty Spaces
     * would be created, and they would all be equal to
     * the original Empty Space, so that the final result
     * would be a collection with only one Empty Space.
     */
    // x
    //// back
    if (allocated_rectangular_cuboid.initial_position().x() > empty_space.initial_position().x()) {
        container.emplace_back(details::cut_space_back(empty_space, allocated_rectangular_cuboid));
    }
    //// front
    if (allocated_rectangular_cuboid.final_position().x() < empty_space.final_position().x()) {
        container.emplace_back(details::cut_space_front(empty_space, allocated_rectangular_cuboid));
    }
    // y
    // right
    if (allocated_rectangular_cuboid.initial_position().y() > empty_space.initial_position().y()) {
        container.emplace_back(details::cut_space_right(empty_space, allocated_rectangular_cuboid));
    }
    //// left
    if (allocated_rectangular_cuboid.final_position().y() < empty_space.final_position().y()) {
        container.emplace_back(details::cut_space_left(empty_space, allocated_rectangular_cuboid));
    }
    // z
    //// bottom
    if (allocated_rectangular_cuboid.initial_position().z() > empty_space.initial_position().z()) {
        container.emplace_back(details::cut_space_bottom(empty_space, allocated_rectangular_cuboid));
    }
    //// top
    if (allocated_rectangular_cuboid.final_position().z() < empty_space.final_position().z()) {
        container.emplace_back(details::cut_space_top(empty_space, allocated_rectangular_cuboid));
    }
    return;
}

} // namespace sclp

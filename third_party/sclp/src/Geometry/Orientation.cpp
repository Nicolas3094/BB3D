#include "sclp/Geometry/Orientation.hpp"

#include <stdexcept>

namespace sclp {

auto to_string(OrientationType const & orientation_type) -> std::string {
    switch (orientation_type) {
    case OrientationType::XYZ:
        return "xyz";
    case OrientationType::YXZ:
        return "yxz";
    case OrientationType::ZYX:
        return "zyx";
    case OrientationType::XZY:
        return "xzy";
    case OrientationType::ZXY:
        return "zxy";
    case OrientationType::YZX:
        return "yzx";
    default:
        throw std::runtime_error(fmt::format("invalid orientation type provided: {}", static_cast<int>(orientation_type)));
    }
}

auto operator<<(std::ostream & os, OrientationType const & orientation_type) -> std::ostream & {
    return os << to_string(orientation_type);
}

EnabledOrientations::EnabledOrientations()
    : enabled_orientations(0) {}

EnabledOrientations::EnabledOrientations(std::vector<OrientationType> const & enabled_orientations)
    : EnabledOrientations() {
    for (auto const & orientation : enabled_orientations) {
        this->enable(orientation);
    }
    return;
}

EnabledOrientations::EnabledOrientations(std::initializer_list<OrientationType> const & enabled_orientations)
    : EnabledOrientations() {
    for (auto const & orientation : enabled_orientations) {
        this->enable(orientation);
    }
    return;
}

auto EnabledOrientations::enable(OrientationType const & orientation) -> void {
    this->enabled_orientations[static_cast<ORIENTATION_DATA_TYPE>(orientation)] = true;
    return;
}

auto EnabledOrientations::operator[](OrientationType const & orientation) const -> bool {
    return this->enabled_orientations[static_cast<ORIENTATION_DATA_TYPE>(orientation)];
}

auto rotate(DiscreteVector const & vector, OrientationType orientation) -> DiscreteVector {
    switch (orientation) {
    case OrientationType::XYZ:
        return DiscreteVector(vector);
    case OrientationType::YXZ:
        return DiscreteVector{vector.y(), vector.x(), vector.z()};
    case OrientationType::ZYX:
        return DiscreteVector{vector.z(), vector.y(), vector.x()};
    case OrientationType::XZY:
        return DiscreteVector{vector.x(), vector.z(), vector.y()};
    case OrientationType::ZXY:
        return DiscreteVector{vector.z(), vector.x(), vector.y()};
    case OrientationType::YZX:
        return DiscreteVector{vector.y(), vector.z(), vector.x()};
    case OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS:
        throw std::runtime_error("Orientation::OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS is not a valid orientation");
    default:
        throw std::runtime_error("Unknown orientation");
    }
    return vector;
}

EnabledOrientations::Iterator::Iterator(EnabledOrientations const & enabled_orientations, ORIENTATION_DATA_TYPE const & current_index)
    : enabled_orientations(enabled_orientations)
    , current_index(current_index) {
    while (!this->enabled_orientations.enabled_orientations[this->current_index] && this->current_index < static_cast<ORIENTATION_DATA_TYPE>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS)) {
        ++this->current_index;
    }
    return;
}

auto EnabledOrientations::Iterator::operator++() -> Iterator & {
    do {
        ++this->current_index;
    } while (!this->enabled_orientations.enabled_orientations[current_index] && this->current_index < static_cast<ORIENTATION_DATA_TYPE>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS));
    return *this;
}

auto EnabledOrientations::Iterator::operator*() const -> value_type {
    return static_cast<OrientationType>(current_index);
}

auto EnabledOrientations::Iterator::operator!=(Iterator const & other) const -> bool {
#ifdef SCLP_DEBUG
    if (&this->enabled_orientations != &other.enabled_orientations) {
        throw std::runtime_error("Iterator::operator!=() called on iterators of different Enabled Orientations");
    }
#endif
    return current_index != other.current_index;
}

auto EnabledOrientations::begin() const -> Iterator {
    Iterator iter(*this, static_cast<ORIENTATION_DATA_TYPE>(0));
    return iter;
}

auto EnabledOrientations::end() const -> Iterator {
    Iterator iter(*this, static_cast<ORIENTATION_DATA_TYPE>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS));
    return iter;
}

} // namespace sclp

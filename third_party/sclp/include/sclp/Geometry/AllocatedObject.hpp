#ifndef SCLP_GEOMETRY_ALLOCATED_OBJECT_HPP_
#define SCLP_GEOMETRY_ALLOCATED_OBJECT_HPP_

#include <ostream>
#include <string>

#include <fmt/core.h>

#include "sclp/Geometry/DiscreteVector.hpp"

namespace sclp {

class AllocatedObject {
public:
    AllocatedObject(DiscreteVector const & initial_position, DiscreteVector const & final_position) noexcept
        : _initial_position(initial_position)
        , _final_position(final_position) {}

    auto initial_position() const noexcept -> DiscreteVector const & { return this->_initial_position; }
    auto final_position() const noexcept -> DiscreteVector const & { return this->_final_position; }

    auto operator==(AllocatedObject const & that) const noexcept -> bool {
        return (this->initial_position() == that.initial_position()) && (this->final_position() == that.final_position());
    }

    friend auto to_json(Json & obj, AllocatedObject const & allocated_object) -> void {
        obj["initial_position"] = allocated_object.initial_position();
        obj["final_position"] = allocated_object.final_position();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, AllocatedObject const & allocated_object) -> std::ostream & {
        return os << std::string(allocated_object);
    }

private:
    DiscreteVector _initial_position;
    DiscreteVector _final_position;
};

} // namespace sclp

#endif

#ifndef SCLP_GEOMETRY_AREA_HPP_
#define SCLP_GEOMETRY_AREA_HPP_

#include <algorithm>
#include <optional>
#include <stdexcept>

#include <fmt/core.h>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/DiscreteVector.hpp"

namespace sclp {

auto project(Plane const & plane, DiscreteVector const & v) -> DiscreteVector2D;

class Area {
public:
    static auto from_measurement(
        Plane const & plane,
        DiscreteVector2D const & measurement,
        DiscreteVector2D const & initial_position,
        DiscreteVector2D::scalar const & orthogonal_position)
        -> Area {
        return Area(plane, initial_position, initial_position + measurement, orthogonal_position);
    }

    static auto from_positions(
        Plane const & plane,
        DiscreteVector2D const & initial_position,
        DiscreteVector2D const & final_position,
        DiscreteVector2D::scalar const & orthogonal_position)
        -> Area {
        return Area(plane, initial_position, final_position, orthogonal_position);
    }

    inline auto plane() const noexcept -> Plane const & { return this->_plane; }
    inline auto initial_position() const noexcept -> DiscreteVector2D const & { return this->_initial_position; }
    inline auto final_position() const noexcept -> DiscreteVector2D const & { return this->_final_position; }
    inline auto measurement() const noexcept -> DiscreteVector2D const & { return this->_measurement; }
    inline auto orthogonal_position() const noexcept -> DiscreteVector2D::scalar const & { return this->_orthogonal_position; }

    auto prod() const noexcept -> DiscreteVector2D::large_scalar {
        auto p = DiscreteVector2D::large_scalar{1};
        p *= this->measurement().template get<0>();
        p *= this->measurement().template get<1>();
        return p;
    }
    auto contains(Area const & that) const -> bool {
        if (this->plane() != that.plane()) {
            throw std::runtime_error(fmt::format("planes do not match: {} != {}", static_cast<int>(this->plane()), static_cast<int>(that.plane())));
        }
        return (this->initial_position() <= that.initial_position())
            && (that.final_position() <= this->final_position());
    }
    auto overlaps(Area const & that) const -> bool {
        if (this->plane() != that.plane()) {
            throw std::runtime_error(fmt::format("planes do not match: {} != {}", static_cast<int>(this->plane()), static_cast<int>(that.plane())));
        }
        return (this->initial_position() < that.final_position())
            && (that.initial_position() < this->final_position());
    }
    auto overlaping_area_with(Area const & that) const -> std::optional<Area> {
        if (!this->overlaps(that)) {
            return std::nullopt;
        } else {
            auto initial_position = DiscreteVector2D{
                std::max(this->initial_position().x(), that.initial_position().x()),
                std::max(this->initial_position().y(), that.initial_position().y())};
            auto final_position = DiscreteVector2D{
                std::min(this->final_position().x(), that.final_position().x()),
                std::min(this->final_position().y(), that.final_position().y())};
            return Area::from_positions(this->plane(), initial_position, final_position, this->orthogonal_position());
        }
    }

    auto operator==(Area const & that) const noexcept {
        return (this->plane() == that.plane())
            && (this->initial_position() == that.initial_position())
            && (this->final_position() == that.final_position())
            && (this->measurement() == that.measurement())
            && (this->orthogonal_position() == that.orthogonal_position());
    }

    friend auto to_json(Json & obj, Area const & area) -> void {
        obj["plane"] = area.plane_str();
        obj["initial_position"] = area.initial_position();
        obj["final_position"] = area.final_position();
        obj["orthogonal_position"] = area.orthogonal_position();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, Area const & area) -> std::ostream & {
        return os << std::string(area);
    }

private:
    Plane _plane;
    DiscreteVector2D _initial_position;
    DiscreteVector2D _final_position;
    DiscreteVector2D _measurement;
    DiscreteVector2D::scalar _orthogonal_position;

    Area(
        Plane const & plane,
        DiscreteVector2D const & initial_position,
        DiscreteVector2D const & final_position,
        DiscreteVector2D::scalar const & orthogonal_position)
        : _plane(plane)
        , _initial_position(initial_position)
        , _final_position(final_position)
        , _measurement(final_position - initial_position)
        , _orthogonal_position(orthogonal_position) {}

    auto plane_str() const noexcept -> char const * {
        switch (this->plane()) {
        case Plane::XY:
            return "xy";
        case Plane::XZ:
            return "xz";
        case Plane::YZ:
            return "yz";
        }
        return "";
    }
};

} // namespace sclp

#endif

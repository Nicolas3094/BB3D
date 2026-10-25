
#ifndef SCLP_GEOMETRY_ORIENTATION_HPP_
#define SCLP_GEOMETRY_ORIENTATION_HPP_

#include <initializer_list>
#include <ostream>
#include <string>

#include <fmt/core.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <iterator>
#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/DiscreteVector.hpp"

namespace sclp {

using ORIENTATION_DATA_TYPE = std::uint8_t;

enum class OrientationType : ORIENTATION_DATA_TYPE {
    XYZ = 0u,
    YXZ,
    ZYX,
    XZY,
    ZXY,
    YZX,
    NUMBER_OF_EXISTENT_ORIENTATIONS,
    ALWAYS_ENABLED = XYZ
};

constexpr std::array<OrientationType, static_cast<std::size_t>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS)> ORIENTATION_TYPE_ALL_VALUES = {
    OrientationType::XYZ,
    OrientationType::YXZ,
    OrientationType::ZYX,
    OrientationType::XZY,
    OrientationType::ZXY,
    OrientationType::YZX,
};

auto to_string(OrientationType const & orientation_type) -> std::string;
auto operator<<(std::ostream & os, OrientationType const & orientation_type) -> std::ostream &;

class EnabledOrientations {
public:
    static auto xyz() -> EnabledOrientations {
        EnabledOrientations enabled_orientations;
        enabled_orientations.enable(OrientationType::ALWAYS_ENABLED);
        return enabled_orientations;
    }

    static auto all() -> EnabledOrientations {
        EnabledOrientations enabled_orientations;
        for (auto const & orientation : ORIENTATION_TYPE_ALL_VALUES) {
            enabled_orientations.enable(orientation);
        }
        return enabled_orientations;
    }

    EnabledOrientations();

    EnabledOrientations(std::vector<OrientationType> const & enabled_orientations);
    EnabledOrientations(std::initializer_list<OrientationType> const & enabled_orientations);

    auto operator[](OrientationType const & orientation) const -> bool;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = OrientationType;
        using difference_type = std::ptrdiff_t;
        using pointer = OrientationType const *;
        using reference = OrientationType const &;

        Iterator(EnabledOrientations const & enabled_orientations, ORIENTATION_DATA_TYPE const & current_index);

        auto operator++() -> Iterator &;
        auto operator*() const -> value_type;
        auto operator!=(Iterator const & other) const -> bool;

    private:
        EnabledOrientations const & enabled_orientations;
        ORIENTATION_DATA_TYPE current_index;
    };

    auto begin() const -> Iterator;
    auto end() const -> Iterator;

    auto size() const -> Index {
        return static_cast<Index>(std::distance(this->begin(), this->end()));
    }

    auto operator==(EnabledOrientations const & that) const -> bool {
        return this->enabled_orientations == that.enabled_orientations;
    }

    auto to_string(bool verbose = false) const -> std::string {
        if (!verbose) {
            auto s = std::string();
            for (const auto & orientation : ORIENTATION_TYPE_ALL_VALUES) {
                s += (*this)[orientation] ? "1" : "0";
            }
            return s;
        } else {
            auto s = std::string();
            s += "{";
            for (const auto & orientation : ORIENTATION_TYPE_ALL_VALUES) {
                if (orientation != OrientationType::ALWAYS_ENABLED) {
                    s += ", ";
                }
                s += fmt::format("{}: {}", sclp::to_string(orientation), (*this)[orientation] ? "true" : "false");
            }
            s += "}";
            return s;
        }
    }

    operator std::string() const {
        return this->to_string(false);
    }

    friend auto operator<<(std::ostream & os, EnabledOrientations const & enabled_orientations) -> std::ostream & {
        return os << std::string(enabled_orientations);
    }

protected:
    auto enable(OrientationType const & orientation) -> void;

private:
    std::bitset<static_cast<std::size_t>(OrientationType::NUMBER_OF_EXISTENT_ORIENTATIONS)> enabled_orientations;
};

auto rotate(DiscreteVector const & vector, OrientationType orientation) -> DiscreteVector;

} // namespace sclp

#endif

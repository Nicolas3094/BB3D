#ifndef SCLP_PACKING_ALLOCATED_SMALL_ITEM_HPP_
#define SCLP_PACKING_ALLOCATED_SMALL_ITEM_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/AllocatedRectangularCuboid.hpp"
#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Packing/SmallItem.hpp"

namespace sclp {

class AllocatedSmallItem : public AllocatedRectangularCuboid {
public:
    AllocatedSmallItem(
        SmallItem const & small_item,
        DiscreteVector const & initial_position,
        OrientationType const & orientation) noexcept
        : AllocatedRectangularCuboid(
              rotate(small_item.measurement(), orientation),
              initial_position)
        , _small_item(&small_item)
        , _orientation(orientation) {}

    auto type() const noexcept -> SmallItem const & { return *this->_small_item; }

    auto orientation() const noexcept -> OrientationType {
        return _orientation;
    }

    auto operator==(AllocatedSmallItem const & that) const noexcept -> bool {
        return AllocatedRectangularCuboid::operator==(that) && (this->_small_item == that._small_item) && (this->orientation() == that.orientation());
    }

    friend auto to_json(Json & obj, AllocatedSmallItem const & allocated_small_item) -> void {
        obj["type"] = allocated_small_item.type();
        obj["orientation"] = allocated_small_item.orientation();
        obj["initial_position"] = allocated_small_item.initial_position();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, AllocatedSmallItem const & allocated_small_item) -> std::ostream & {
        return os << std::string(allocated_small_item);
    }

private:
    SmallItem const * _small_item;
    OrientationType _orientation;
};

} // namespace sclp

#endif

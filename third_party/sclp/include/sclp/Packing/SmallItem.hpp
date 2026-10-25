#ifndef SCLP_PACKING_SMALL_ITEM_TYPE_HPP_
#define SCLP_PACKING_SMALL_ITEM_TYPE_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Orientation.hpp"
#include "sclp/Geometry/RectangularCuboid.hpp"

namespace sclp {

class SmallItem : public RectangularCuboid {
public:
    SmallItem(
        Index const & index,
        Quantity const & quantity,
        DiscreteVector const & measurement,
        EnabledOrientations orientation = EnabledOrientations::xyz())
        : RectangularCuboid(measurement)
        , _index(index)
        , _quantity(quantity)
        , _orientation(orientation) {}

    auto index() const noexcept -> Index const & { return this->_index; }

    auto quantity() const noexcept -> Quantity const & { return this->_quantity; }

    auto orientation() const noexcept -> EnabledOrientations const & { return _orientation; }

    auto operator==(SmallItem const & that) const noexcept -> bool {
        return RectangularCuboid::operator==(that) && (this->index() == that.index()) && (this->quantity() == that.quantity()) && (this->orientation() == that.orientation());
    }

    friend auto to_json(Json & obj, SmallItem const & small_item) -> void {
        obj["quantity"] = small_item.quantity();
        obj["measurement"] = small_item.measurement();
        obj["orientation"] = small_item.orientation();
        return;
    }

    operator std::string() const {
        return Json(*this).dump();
    }

    friend auto operator<<(std::ostream & os, SmallItem const & small_item) -> std::ostream & {
        return os << std::string(small_item);
    }

private:
    Index _index;
    Quantity _quantity;
    EnabledOrientations _orientation;
};

} // namespace sclp

#endif

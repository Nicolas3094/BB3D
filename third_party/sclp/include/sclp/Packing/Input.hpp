#ifndef SCLP_PACKING_INPUT_HPP_
#define SCLP_PACKING_INPUT_HPP_

#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Packing/LargeObject.hpp"
#include "sclp/Packing/SmallItem.hpp"

namespace sclp {

struct SmallItemAsKeyCompare {
    auto operator()(SmallItem const * const lhs, SmallItem const * const rhs) const -> bool {
        return lhs->index() < rhs->index();
    }
};

using QuantityMap = boost::container::flat_map<SmallItem const *, Quantity, SmallItemAsKeyCompare>;

class Input {
public:
    Input(LargeObject && large_object, std::vector<SmallItem> && small_items)
        : _large_object{std::move(large_object)}
        , _small_items{std::move(small_items)} {
        this->check_indices_of_small_items();
        this->update_aggregate_properties();
        return;
    }

    auto large_object() const noexcept -> LargeObject const & { return _large_object; };

    auto small_items() const noexcept -> std::vector<SmallItem> const & { return _small_items; }

    auto quantity_map() const noexcept -> QuantityMap { return _quantity_map; }

    auto total_quantity() const noexcept -> Quantity { return _total_quantity; }

    auto total_volume() const noexcept -> Volume { return _total_volume; }

private:
    LargeObject _large_object;
    std::vector<SmallItem> _small_items;

    QuantityMap _quantity_map;
    Quantity _total_quantity;
    Volume _total_volume;

    auto check_indices_of_small_items() const -> void {
        auto const & number_of_small_item_types = this->small_items().size();
        // small items must have indices from 0 to N-1, in that order
        // where N is the number of small item types
        for (auto index = Index(0); index < number_of_small_item_types; ++index) {
            auto const & small_item = this->small_items()[index];
            if (small_item.index() != index) {
                throw std::runtime_error(fmt::format(
                    "the index of the small item at position {} is {}, but they should be equal",
                    index,
                    small_item.index()));
            }
        }
    }

    auto update_aggregate_properties() noexcept -> void {
        this->_total_quantity = 0;
        this->_total_volume = 0;
        for (auto const & small_item : this->_small_items) {
            this->_quantity_map[&small_item] = small_item.quantity();
            this->_total_quantity += small_item.quantity();
            this->_total_volume += small_item.quantity() * small_item.measurement().prod();
        }
        return;
    }
};

} // namespace sclp

#endif

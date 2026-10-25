#ifndef SCLP_CONSTRAINT_BASIC_QUANTITY_CONSTRAINTS_HPP_
#define SCLP_CONSTRAINT_BASIC_QUANTITY_CONSTRAINTS_HPP_

#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class QuantityConstraint {
public:
    using QuantityMap = std::map<const SmallItem *, Quantity>;

    struct Result {
        QuantityMap quantity;
        QuantityMap exceeded_quantity;
        bool valid;
    };

    static auto check(Packing const & packing) -> Result {
        auto result = Result{};
        result.quantity = quantity_map(packing);
        result.exceeded_quantity = find_exceeded_quantities(result.quantity);
        result.valid = result.exceeded_quantity.empty();
        return result;
    }

private:
    static auto quantity_map(const Packing & packing) -> QuantityMap {
        auto quantity_map = QuantityMap{};
        for (const auto & allocated_small_item : packing.allocated_small_items()) {
            const auto & small_item = allocated_small_item.type();
            if (!quantity_map.contains(&small_item)) {
                quantity_map[&small_item] = 0;
            }
            quantity_map[&small_item] += 1;
        }
        return quantity_map;
    }

    static auto find_exceeded_quantities(const QuantityMap & quantity_map) -> QuantityMap {
        auto exceeded = QuantityMap{};
        for (const auto & [small_item, quantity] : quantity_map) {
            if (quantity > small_item->quantity()) {
                exceeded.insert({small_item, quantity});
            }
        }
        return exceeded;
    }
};

} // namespace sclp

#endif

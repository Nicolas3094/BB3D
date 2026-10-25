#ifndef SCLP_CONSTRAINT_ITEM_RELATED_ORIENTATION_CONSTRAINT_CONSTRAINT_HPP_
#define SCLP_CONSTRAINT_ITEM_RELATED_ORIENTATION_CONSTRAINT_CONSTRAINT_HPP_

#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class OrientationConstraint {
public:
    struct Result {
        std::vector<const AllocatedSmallItem *> invalid_orientations;
        bool valid;
    };

    static auto check(Packing const & packing) -> Result {
        auto result = Result{};
        result.invalid_orientations = find_invalid_orientations(packing);
        result.valid = result.invalid_orientations.empty();
        return result;
    }

private:
    static auto find_invalid_orientations(const Packing & packing) -> std::vector<const AllocatedSmallItem *> {
        auto invalid = std::vector<const AllocatedSmallItem *>{};
        for (const auto & allocated_small_item : packing.allocated_small_items()) {
            const auto & enabled_orientations = allocated_small_item.type().orientation();
            const auto & orientation_allocated = allocated_small_item.orientation();
            if (!enabled_orientations[orientation_allocated]) {
                invalid.push_back(&allocated_small_item);
            }
        }
        return invalid;
    }
};

} // namespace sclp

#endif

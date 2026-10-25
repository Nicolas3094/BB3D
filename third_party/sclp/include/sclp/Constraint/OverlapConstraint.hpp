#ifndef SCLP_CONSTRAINT_BASIC_OVERLAP_CONSTRAINT_HPP_
#define SCLP_CONSTRAINT_BASIC_OVERLAP_CONSTRAINT_HPP_

#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class OverlapConstraint {
public:
    using OverlapPairsList = std::vector<std::pair<const AllocatedSmallItem *, const AllocatedSmallItem *>>;

    struct Result {
        bool valid;
        OverlapPairsList overlaps;
    };

    static auto check(Packing const & packing) -> Result {
        auto result = Result();
        result.overlaps = find_overlaps(packing);
        result.valid = result.overlaps.empty();
        return result;
    }

private:
    static auto find_overlaps(const Packing & packing) -> OverlapPairsList {
        auto overlaps = OverlapPairsList();
        for (Index i = 0; i < packing.allocated_small_items().size(); ++i) {
            for (Index j = i + 1; j < packing.allocated_small_items().size(); ++j) {
                const auto & allocated_small_item = packing.allocated_small_items()[i];
                const auto & other_allocated_small_item = packing.allocated_small_items()[j];
                if (allocated_small_item.overlaps(other_allocated_small_item)) {
                    overlaps.emplace_back(&allocated_small_item, &other_allocated_small_item);
                }
            }
        }
        return overlaps;
    }
};

} // namespace sclp

#endif

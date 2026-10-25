#ifndef SCLP_CONSTRAINT_BASIC_LARGE_OBJECT_SPACE_CONSTRAINT_HPP_
#define SCLP_CONSTRAINT_BASIC_LARGE_OBJECT_SPACE_CONSTRAINT_HPP_

#include "sclp/Packing/AllocatedSmallItem.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class LargeObjectSpaceConstraint {
public:
    struct Result {
        bool valid;
        std::vector<const AllocatedSmallItem *> outside;
    };

    static auto check(const Input & input, Packing const & packing) -> Result {
        auto result = Result();
        result.outside = find_allocated_small_items_outside_large_object(input.large_object(), packing);
        result.valid = result.outside.empty();
        return result;
    }

private:
    static auto find_allocated_small_items_outside_large_object(const LargeObject & large_object, const Packing & packing) -> std::vector<const AllocatedSmallItem *> {
        auto outside = std::vector<const AllocatedSmallItem *>{};
        for (const auto & allocated_small_item : packing.allocated_small_items()) {
            if (!(allocated_small_item.final_position() <= large_object.measurement())) {
                outside.push_back(&allocated_small_item);
            }
        }
        return outside;
    }
};

} // namespace sclp

#endif

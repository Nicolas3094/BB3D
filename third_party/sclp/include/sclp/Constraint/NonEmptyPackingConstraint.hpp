#ifndef SCLP_CONSTRAINT_BASIC_NON_EMPTY_PACKING_CONSTRAINTS_HPP_
#define SCLP_CONSTRAINT_BASIC_NON_EMPTY_PACKING_CONSTRAINTS_HPP_

#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class NonEmptyPackingConstraint {
public:
    static auto check(Input const & input, Packing const & packing) -> bool {
        return input.small_items().empty() == packing.allocated_small_items().empty();
    }
};

} // namespace sclp

#endif

#ifndef SCLP_CONSTRAINT_SETS_ALL_CONSTRAINTS_HPP_
#define SCLP_CONSTRAINT_SETS_ALL_CONSTRAINTS_HPP_

#include "sclp/Constraint/LargeObjectSpaceConstraint.hpp"
#include "sclp/Constraint/NonEmptyPackingConstraint.hpp"
#include "sclp/Constraint/OrientationConstraint.hpp"
#include "sclp/Constraint/OverlapConstraint.hpp"
#include "sclp/Constraint/QuantityConstraint.hpp"
#include "sclp/Packing/Input.hpp"
#include "sclp/Packing/Packing.hpp"

namespace sclp {

class AllConstraints {
public:
    static auto check(Input const & input, Packing const & packing) -> bool {
        auto valid = true;
        if (valid) { valid = valid && NonEmptyPackingConstraint::check(input, packing); }
        if (valid) { valid = valid && QuantityConstraint::check(packing).valid; }
        if (valid) { valid = valid && LargeObjectSpaceConstraint::check(input, packing).valid; }
        if (valid) { valid = valid && OrientationConstraint::check(packing).valid; }
        if (valid) { valid = valid && OverlapConstraint::check(packing).valid; }
        return valid;
    }
};

} // namespace sclp

#endif

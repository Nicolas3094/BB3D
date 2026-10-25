#ifndef SCLP_ALLOCATION_FEATURES_HPP_
#define SCLP_ALLOCATION_FEATURES_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

namespace sclp {

struct Features {
    static constexpr auto NUMBER_OF_VALUES = std::size_t(5);

    double V = 0.0; // volume of the block
    double C = 0.0; // relative pseudo-contact area; 1.0: full contanct, 0.0: no contact
    double L = 0.0; // empty space relative volume loss; 1.0: no loss, 0.0: full loss
    double N = 0.0; // number of items that compose the block
    double F = 0.0; // fill rate of the large object

    operator std::string() const {
        return fmt::format("[{}, {}, {}, {}, {}]", V, C, L, N, F);
    }
};

} // namespace sclp

#endif

#ifndef SCLP_ALLOCATION_CONFIGURATION_HPP_
#define SCLP_ALLOCATION_CONFIGURATION_HPP_

#include <memory>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/PseudoContact.hpp"

namespace sclp {

struct Configuration {
    using Sptr = std::shared_ptr<Configuration>;

    static constexpr auto NUMBER_OF_VALUES = std::size_t(4);

    double alpha; // impact of the relative contact area
    double beta; // impact of the volume loss
    double gamma; // impact of the number of items
    double delta; // relative contact tolerance, to compute contact between blocks

    PseudoContact pseudo_contact;

    Configuration(
        double const & alpha,
        double const & beta,
        double const & gamma,
        double const & delta)
        : alpha(alpha)
        , beta(beta)
        , gamma(gamma)
        , delta(delta)
        , pseudo_contact(delta) {}

    operator std::string() const {
        return fmt::format("[{}, {}, {}, {}]", alpha, beta, gamma, delta);
    }
};

} // namespace sclp

#endif

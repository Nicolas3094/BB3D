#ifndef SCLP_TYPES_TIMER_HPP_
#define SCLP_TYPES_TIMER_HPP_

#include <limits>
#include <stdexcept>

#include <boost/timer/timer.hpp>

#include <fmt/core.h>

namespace sclp {

using Seconds = double;

class TimeLimitException : public std::runtime_error {
public:
    TimeLimitException(double elapsed_time, double time_limit)
        : std::runtime_error(fmt::format("running time limit {} reached with value {}", time_limit, elapsed_time)) {}
};

class Timer {
public:
    Timer(Seconds time_limit = std::numeric_limits<Seconds>::max());

    auto time_limit() const noexcept -> Seconds;

    auto resume() -> void;
    auto stop() -> void;
    auto restart() -> void;
    auto check() -> void;

    auto timeout() -> bool;
    auto remaining() -> Seconds;
    auto running_elapsed() const -> Seconds;
    auto elapsed() const -> Seconds;
    auto elapsed_wall() const -> Seconds;

private:
    boost::timer::cpu_timer _timer;
    Seconds _time_limit;
};

} // namespace sclp

#endif

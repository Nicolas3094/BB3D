#include "sclp/Types/Timer.hpp"

#include <stdexcept>

namespace sclp {

constexpr auto NANOSECOND_AS_SECOND = 1.0e-9; // 1 ns = 1.0e-9 s

Timer::Timer(Seconds time_limit)
    : _time_limit(time_limit) {
    this->restart();
    return;
}

auto Timer::time_limit() const noexcept -> Seconds {
    return this->_time_limit;
}

auto Timer::resume() -> void {
    this->_timer.resume();
    return;
}

auto Timer::stop() -> void {
    this->_timer.stop();
    return;
}

auto Timer::restart() -> void {
    this->_timer.start();
    this->_timer.stop();
    return;
}

auto Timer::check() -> void {
    if (this->timeout()) {
        this->stop();
        throw TimeLimitException(this->elapsed(), this->time_limit());
    }
}

auto Timer::timeout() -> bool {
    return this->remaining() <= 0.0;
}

auto Timer::remaining() -> Seconds {
    return this->time_limit() - this->running_elapsed();
}

auto Timer::running_elapsed() const -> Seconds {
    return NANOSECOND_AS_SECOND * (this->_timer.elapsed().user + this->_timer.elapsed().system);
}

auto Timer::elapsed() const -> Seconds {
    if (!this->_timer.is_stopped()) {
        throw std::runtime_error("timer is not stopped");
    }
    return this->running_elapsed();
}

auto Timer::elapsed_wall() const -> Seconds {
    if (!this->_timer.is_stopped()) {
        throw std::runtime_error("timer is not stopped");
    }
    return NANOSECOND_AS_SECOND * (this->_timer.elapsed().wall);
}

} // namespace sclp

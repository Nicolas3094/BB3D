#ifndef SCLP_ALLOCATION_SOLUTION_HISTORY_HPP_
#define SCLP_ALLOCATION_SOLUTION_HISTORY_HPP_

#include <vector>

#include "sclp/Types.hpp" // IWYU pragma: keep

namespace sclp {

struct SolutionHistory {
    struct Entry {
        Seconds time;
        Index branching_factor;
        Volume volume_usage;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Entry, time, branching_factor, volume_usage);
    };

    std::vector<Entry> entries;

    auto add(Entry entry) -> void {
        this->entries.push_back(entry);
        return;
    }

    auto extend(SolutionHistory const & other, Seconds initial_time = 0.0) -> void {
        for (auto const & entry : other.entries) {
            this->entries.push_back({entry.time + initial_time,
                entry.branching_factor,
                entry.volume_usage});
        }
        return;
    }
};

} // namespace sclp

#endif

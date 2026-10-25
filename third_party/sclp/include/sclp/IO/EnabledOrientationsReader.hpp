
#ifndef SCLP_IO_ENABLED_ORIENTATIONS_READER_HPP_
#define SCLP_IO_ENABLED_ORIENTATIONS_READER_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/Orientation.hpp"
#include "sclp/IO/JsonKey.hpp"

namespace sclp {

struct EnabledOrientationsReader {
    EnabledOrientationsReader() = delete;

    static auto read(const Json & json) -> EnabledOrientations {
        if (json.contains(JsonKey::CONSTRAINT) && json.at(JsonKey::CONSTRAINT).contains(JsonKey::ORIENTATION)) {
            auto const & data = json.at(JsonKey::CONSTRAINT).at(JsonKey::ORIENTATION);
            std::vector<OrientationType> values;
            for (auto const & orientation : ORIENTATION_TYPE_ALL_VALUES) {
                auto const orientation_str = to_string(orientation);
                if (data.contains(orientation_str) && data.at(orientation_str).get<bool>()) {
                    values.push_back(orientation);
                }
            }
            auto enabled_orientations = EnabledOrientations(values);
#ifdef SCLP_DEBUG
            if (!enabled_orientations[OrientationType::ALWAYS_ENABLED]) {
                throw std::invalid_argument("There is one orientation which must always be enabled, but it is disabled");
            }
#endif
            return enabled_orientations;
        } else {
            return EnabledOrientations::xyz();
        }
    }
};

} // namespace sclp

#endif

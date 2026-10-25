#ifndef SCLP_IO_DISCRETE_VECTOR_READER_HPP_
#define SCLP_IO_DISCRETE_VECTOR_READER_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Geometry/DiscreteVector.hpp"
#include "sclp/IO/JsonKey.hpp"

namespace sclp {

struct DiscreteVectorReader {
    DiscreteVectorReader() = delete;

    static auto read(const Json & data) -> DiscreteVector {
        return DiscreteVector{
            data.at(JsonKey::X).get<CoordinateType>(),
            data.at(JsonKey::Y).get<CoordinateType>(),
            data.at(JsonKey::Z).get<CoordinateType>()};
    }
};

} // namespace sclp

#endif

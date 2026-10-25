#ifndef SCLP_IO_LARGE_OBJECT_READER_HPP_
#define SCLP_IO_LARGE_OBJECT_READER_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/IO/DiscreteVectorReader.hpp"
#include "sclp/IO/JsonKey.hpp"
#include "sclp/Packing/LargeObject.hpp"

namespace sclp {

struct LargeObjectReader {
    LargeObjectReader() = delete;

    static auto read(const Json & data) -> LargeObject {
        auto measurement = DiscreteVectorReader::read(data.at(JsonKey::MEASUREMENT));
        return LargeObject(measurement);
    }
};

} // namespace sclp

#endif

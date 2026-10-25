#ifndef SCLP_IO_INPUT_READER_HPP_
#define SCLP_IO_INPUT_READER_HPP_

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/IO/JsonKey.hpp"
#include "sclp/IO/LargeObjectReader.hpp"
#include "sclp/IO/SmallItemReader.hpp"
#include "sclp/Packing/Input.hpp"

namespace sclp {

struct InputReader {
    InputReader() = delete;

    static auto read(const Json & data) -> Input {
        return Input(
            LargeObjectReader::read(data.at(JsonKey::LARGE_OBJECT)),
            SmallItemReader::read_collection(data.at(JsonKey::SMALL_ITEMS)));
    }
};

} // namespace sclp

#endif

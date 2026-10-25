#ifndef SCLP_FILE_LOADER_HPP_
#define SCLP_FILE_LOADER_HPP_

#include <fstream>
#include <stdexcept>
#include <string>

#include "sclp/Types.hpp" // IWYU pragma: keep

namespace sclp {

class FileLoader {
public:
    static auto json(const std::string file_path) -> Json {
        auto file = std::ifstream(file_path, std::ifstream::in);
        if (!file.good()) {
            throw std::runtime_error("problem loading file " + file_path);
        }
        auto data = Json::parse(file);
        file.close();
        return data;
    }
};

} // namespace sclp

#endif

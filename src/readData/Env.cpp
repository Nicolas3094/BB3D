#include "Env.hpp"
#include <stdexcept>

namespace
{
std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return {};

    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}
}

std::string getEnv(std::string parameter, const std::string &path)
{
    std::ifstream file(path);

    if (!file)
        throw std::runtime_error("Cannot open environment file: " + path);

    std::string line;
    bool first = true;

    while (std::getline(file, line))
    {
        if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        first = false;
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        const auto equal = line.find('=');

        if (equal == std::string::npos || trim(line.substr(0, equal)) != parameter)
            continue;

        auto value = trim(line.substr(equal + 1));

        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                  (value.front() == '\'' && value.back() == '\'')))
            value = value.substr(1, value.size() - 2);
        if (value.empty())
            throw std::invalid_argument("Empty environment parameter: " + parameter);

        return value;
    }
    if (file.bad())
        throw std::runtime_error("Cannot read environment file: " + path);

    throw std::invalid_argument("Environment parameter not found: " + parameter);
}

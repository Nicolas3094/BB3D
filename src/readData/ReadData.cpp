#include "ReadData.hpp"
#include "BRInstanceReader.hpp"
#include <cerrno>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace
{
void malformed(const std::string &message)
{
    throw std::invalid_argument("Dataset: " + message);
}

template <typename T> std::vector<T> readResultNumbers(const std::string &path)
{
    errno = 0;
    std::ifstream file(path);

    if (!file.is_open())
    {
        if (errno == ENOENT)
            return {};

        throw std::runtime_error("Cannot open results file: " + path);
    }
    std::vector<T> values;
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream row(line);
        T value;
        std::string extra;

        if (!(row >> value) || (row >> extra) || !std::isfinite(static_cast<double>(value)))
            throw std::invalid_argument("Invalid result row in: " + path);

        values.push_back(value);
    }
    if (file.bad())
        throw std::runtime_error("Cannot read results file: " + path);

    return values;
}
}

vector<DatasetBinBacking> readData(const string path)
{
    // Dispatch by the header, not by extension: BR files have a single count.
    std::ifstream probe(path);

    if (!probe)
        throw std::runtime_error("Cannot open dataset: " + path);

    std::string line;
    bool first = true;

    while (std::getline(probe, line))
    {
        if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        first = false;
        const auto header = getNumbersFromLines(line);

        if (header.empty())
            continue;

        if (header.size() == 1)
            return BRInstanceReader().read(path);

        break;
    }
    if (probe.bad())
        throw std::runtime_error("Cannot read dataset: " + path);

    const auto rows = getNumberListFile(path);

    if (rows.empty() || rows[0].size() != 2)
        malformed("missing instance/item count header: " + path);
    const int instances = rows[0][0], expectedItems = rows[0][1];

    if (instances <= 0 || expectedItems <= 0 || static_cast<size_t>(instances) > (rows.size() - 1) / 4)
        malformed("invalid counts or incomplete instances: " + path);
    size_t cursor = 1;
    auto nextRow = [&](size_t width) -> const std::vector<int> &
    {
        if (cursor >= rows.size() || rows[cursor].size() != width)
            malformed("missing or malformed record at row " + std::to_string(cursor + 1) + ": " + path);

        return rows[cursor++];
    };
    std::vector<DatasetBinBacking> datasets;
    datasets.reserve(instances);
    std::unordered_set<int> instanceIds;

    for (int i = 0; i < instances; ++i)
    {
        DatasetBinBacking dataset{};
        const auto &identity = nextRow(2);

        if (identity[0] <= 0 || !instanceIds.insert(identity[0]).second)
            malformed("instance IDs must be positive and unique");
        dataset.id = identity[0];
        dataset.PSeed = identity[1];
        dataset.rotationWay = ROTATION_WAY::ZERO_WAY;
        const auto &dimensions = nextRow(3);

        if (dimensions[0] <= 0 || dimensions[1] <= 0 || dimensions[2] <= 0)
            malformed("container dimensions must be positive");
        dataset.bin.setLargo(dimensions[0]).setAncho(dimensions[1]).setAlto(dimensions[2]);
        dataset.bin.getDimensions().getVolumen();
        const int typeCount = nextRow(1)[0];

        if (typeCount <= 0 || typeCount > expectedItems)
            malformed("invalid item type count");
        std::unordered_set<int> types;

        for (int type = 0; type < typeCount; ++type)
        {
            const auto &record = nextRow(8);

            if (record[0] <= 0 || !types.insert(record[0]).second)
                malformed("item type IDs must be positive and unique");
            const int quantity = record[1];

            if (quantity <= 0 || static_cast<size_t>(quantity) >
                                     static_cast<size_t>(expectedItems) - dataset.totalItems.size())
                malformed("item quantity exceeds the instance count");
            if (record[3] <= 0 || record[5] <= 0 || record[7] <= 0)
                malformed("item dimensions must be positive");
            for (int permission : {record[2], record[4], record[6]})
                if (permission != 0 && permission != 1)
                    malformed("rotation permissions must be 0 or 1");
            ItemBin item = ItemBin::Build()
                               .setId(record[0])
                               .setTipo(record[0])
                               .setLargo(record[3])
                               .setAncho(record[5])
                               .setAlto(record[7])
                               .setRotations(Punto::Build(record[2], record[4], record[6]));
            item.getVolumen();
            dataset.itemsByTpe.push_back(item);

            for (int copy = 0; copy < quantity; ++copy)
                dataset.totalItems.push_back(item.setId(static_cast<int>(dataset.totalItems.size() + 1)));
        }
        if (dataset.totalItems.size() != static_cast<size_t>(expectedItems))
            malformed("instance has an incomplete item list");
        datasets.push_back(std::move(dataset));
    }
    if (cursor != rows.size())
        malformed("unexpected records after the final instance");

    return datasets;
}

vector<vector<int>> getNumberListFile(const string path)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("Cannot open dataset: " + path);

    std::vector<std::vector<int>> rows;
    std::string line;
    bool first = true;

    while (std::getline(file, line))
    {
        if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        first = false;
        auto values = getNumbersFromLines(line);

        if (!values.empty())
            rows.push_back(std::move(values));
    }
    if (file.bad())
        throw std::runtime_error("Cannot read dataset: " + path);

    return rows;
}

vector<int> getNumbersFromLines(string line)
{
    std::istringstream input(line);
    std::vector<int> values;
    std::string token;

    while (input >> token)
    {
        size_t consumed = 0;
        long long value;
        try
        {
            value = std::stoll(token, &consumed);
        }
        catch (const std::exception &)
        {
            throw std::invalid_argument("Invalid dataset integer: " + token);
        }
        if (consumed != token.size() || value < std::numeric_limits<int>::min() ||
            value > std::numeric_limits<int>::max())
            throw std::invalid_argument("Invalid dataset integer: " + token);

        values.push_back(static_cast<int>(value));
    }
    return values;
}

vector<double> getFloatNumberListFile(const string path)
{
    return readResultNumbers<double>(path);
}

vector<std::int64_t> getIntegerNumberListFile(const string path)
{
    return readResultNumbers<std::int64_t>(path);
}

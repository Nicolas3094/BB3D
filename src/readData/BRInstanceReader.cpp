#include "BRInstanceReader.hpp"
#include "ReadData.hpp"
#include <fstream>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace
{
class Records
{
  public:
    Records(std::istream &input, const std::string &source) : input(input), source(source) {}

    bool next(std::vector<int> &values)
    {
        std::string line;

        while (std::getline(input, line))
        {
            ++lineNumber;

            if (lineNumber == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
                line.erase(0, 3);
            try
            {
                values = getNumbersFromLines(line);
            }
            catch (const std::invalid_argument &error)
            {
                fail(error.what());
            }
            if (!values.empty())
                return true;
        }
        if (input.bad() || !input.eof())
            throw std::runtime_error(location() + "cannot read input");

        return false;
    }

    std::vector<int> require(std::size_t columns, const char *description)
    {
        std::vector<int> values;

        if (!next(values))
            fail(std::string("missing ") + description);
        if (values.size() != columns)
            fail(std::string("expected ") + std::to_string(columns) + " columns for " + description);

        return values;
    }

    [[noreturn]] void fail(const std::string &message) const
    {
        throw std::invalid_argument(location() + message);
    }

  private:
    std::istream &input;
    const std::string &source;
    std::size_t lineNumber = 0;

    std::string location() const
    {
        return "BR " + source + ":" + std::to_string(lineNumber) + ": ";
    }
};
}

BRInstanceReader::BRInstanceReader(std::size_t maximumItemsPerInstance)
    : maximumItemsPerInstance(maximumItemsPerInstance)
{
    if (maximumItemsPerInstance == 0 ||
        maximumItemsPerInstance > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::invalid_argument("BR item expansion limit must be between 1 and INT_MAX.");
}

std::vector<DatasetBinBacking> BRInstanceReader::read(const std::string &path) const
{
    std::ifstream input(path);

    if (!input)
        throw std::runtime_error("Cannot open BR dataset: " + path);

    return read(input, path);
}

std::vector<DatasetBinBacking> BRInstanceReader::read(std::istream &input, const std::string &source) const
{
    Records records(input, source);
    const int count = records.require(1, "instance count")[0];

    if (count <= 0)
        records.fail("instance count must be positive");
    std::vector<DatasetBinBacking> result;
    std::unordered_set<int> instanceIds;
    // Do not reserve from an untrusted header: truncated huge counts fail on
    // the first missing record without allocating the declared dataset size.
    for (int instance = 0; instance < count; ++instance)
    {
        DatasetBinBacking data{};
        const auto identity = records.require(2, "instance ID and seed");

        if (identity[0] <= 0 || !instanceIds.insert(identity[0]).second)
            records.fail("instance IDs must be positive and unique");
        data.id = identity[0];
        data.PSeed = identity[1];
        data.rotationWay = ROTATION_WAY::ZERO_WAY;
        const auto dimensions = records.require(3, "container dimensions");

        for (int dimension : dimensions)
            if (dimension <= 0)
                records.fail("container dimensions must be positive");
        data.bin.setLargo(dimensions[0]).setAncho(dimensions[1]).setAlto(dimensions[2]);
        try
        {
            data.bin.getDimensions().getVolumen();
        }
        catch (const std::overflow_error &error)
        {
            records.fail(error.what());
        }
        const int types = records.require(1, "item type count")[0];

        if (types <= 0 || static_cast<std::size_t>(types) > maximumItemsPerInstance)
            records.fail("invalid item type count or expansion limit exceeded");
        std::unordered_set<int> typeIds;

        for (int type = 0; type < types; ++type)
        {
            const auto row = records.require(8, "item type");

            if (row[0] <= 0 || !typeIds.insert(row[0]).second)
                records.fail("item type IDs must be positive and unique");
            if (row[1] <= 0 || row[3] <= 0 || row[5] <= 0)
                records.fail("item dimensions must be positive");
            for (int flag : {row[2], row[4], row[6]})
                if (flag != 0 && flag != 1)
                    records.fail("rotation permissions must be 0 or 1");
            if (row[7] <= 0 ||
                static_cast<std::size_t>(row[7]) > maximumItemsPerInstance - data.totalItems.size())
                records.fail("item quantity must be positive and fit the expansion limit");
            ItemBin item = ItemBin::Build()
                               .setId(row[0])
                               .setTipo(row[0])
                               .setLargo(row[1])
                               .setAncho(row[3])
                               .setAlto(row[5])
                               .setRotations(Punto::Build(row[2], row[4], row[6]));
            try
            {
                item.getVolumen();
            }
            catch (const std::overflow_error &error)
            {
                records.fail(error.what());
            }
            data.itemsByTpe.push_back(item);

            for (int copy = 0; copy < row[7]; ++copy)
                data.totalItems.push_back(item.setId(static_cast<int>(data.totalItems.size() + 1)));
        }
        result.push_back(std::move(data));
    }
    std::vector<int> extra;

    if (records.next(extra))
        records.fail("unexpected records after the final instance");

    return result;
}

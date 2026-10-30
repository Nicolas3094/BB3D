#ifndef BR_INSTANCE_READER_H
#define BR_INSTANCE_READER_H

#include "DataSetBPP.hpp"
#include <cstddef>
#include <istream>
#include <string>
#include <vector>

// Original Bischoff/Ratcliff format: one instance-count header, then
// ID/seed, container dimensions, type count, and eight columns per type:
// type, length, allowX, width, allowY, height, allowZ, quantity.
// Each instance may contain a different total number of boxes.
class BRInstanceReader
{
  public:
    // Configurable expansion limit; IDs must also fit the project's int genes.
    explicit BRInstanceReader(std::size_t maximumItemsPerInstance = 1000000);
    std::vector<DatasetBinBacking> read(const std::string &path) const;
    std::vector<DatasetBinBacking> read(std::istream &input, const std::string &source = "<stream>") const;

  private:
    std::size_t maximumItemsPerInstance;
};

#endif

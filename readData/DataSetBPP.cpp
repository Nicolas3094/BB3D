#include "DataSetBPP.hpp"

std::ostream &operator<<(std::ostream &COUT, DatasetBinBacking &dataset)
{
    COUT << "DATASET BIN PACKING\n"
         << "Id: " << dataset.id << "\n"
         << "p seed: " << dataset.PSeed << "\n"
         << "Bin: " << dataset.bin << "\n"
         << "Items By Type: " << dataset.itemsByTpe << "\n"
         << "Total items: " << dataset.totalItems << "\n";

    return COUT;
}
#ifndef READ_DATA_H
#define READ_DATA_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <fstream>
#include <vector>
#include <typeinfo>
#include "../binPacking/Bin.hpp"
#include "DataSetBPP.hpp"
#include <stdexcept>
#include <cstdint>
#include "VectorFile.hpp"

using namespace std;

// Auto-detects P1-P5 (two header integers) or original BR (one header integer).
vector<DatasetBinBacking> readData(const std::string);
vector<int> getNumbersFromLines(string);
vector<vector<int>> getNumberListFile(const string);
vector<double> getFloatNumberListFile(const string);
// Result readers return all rows; missing result files return an empty vector.
vector<std::int64_t> getIntegerNumberListFile(const string);
#endif

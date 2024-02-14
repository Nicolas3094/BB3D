#ifndef READ_DATA_H
#define READ_DATA_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <fstream>
#include <vector>
#include <typeinfo>
#include "../src/Bin.hpp"
#include "DataSetBPP.hpp"
#include <stdexcept>
#include "VectorFile.hpp"

using namespace std;

vector<DatasetBinBacking> readData(const std::string);
vector<int> getNumbersFromLines(string);
vector<vector<int>> getNumberListFile(const string);

#endif
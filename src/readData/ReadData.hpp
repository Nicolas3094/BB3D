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
#include "VectorFile.hpp"

using namespace std;

vector<DatasetBinBacking> readData(const std::string);
vector<int> getNumbersFromLines(string);
vector<vector<int>> getNumberListFile(const string);
vector<double> getFloatNumberListFile(const string);
vector<long int> getIntegerNumberListFile(const string);
#endif
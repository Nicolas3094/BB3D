#include "ReadData.hpp"

vector<DatasetBinBacking> readData(const string path)
{
    Bin bin;
    vector<vector<int>> numberListByLine = getNumberListFile(path);
    int totalNumberOfInstances = numberListByLine[0][0];
    int totalNumberOfItems = numberListByLine[0][1];
    numberListByLine.erase(numberListByLine.begin());

    vector<DatasetBinBacking> dataSet(totalNumberOfInstances);

    int countInstance = 0;
    int vectorID = 1;
    int numberOftypes = 0;

    for (auto vectorNumber : numberListByLine)
    {
        switch (getVectorFileFromValue(vectorNumber.size()))
        {
        case ID_SEED:
            dataSet[countInstance].id = vectorNumber[0];
            dataSet[countInstance].PSeed = vectorNumber[1];
            break;
        case BIN_DIMENSIONS:
            dataSet[countInstance].bin.setLargo(vectorNumber[0]).setAncho(vectorNumber[1]).setAlto(vectorNumber[2]);
            break;
        case NUMBER_OF_TYPES:
            numberOftypes = vectorNumber[0];
            break;
        case ITEM_DATA:
            ItemBin itemBin;
            itemBin.setTipo(vectorNumber[0])
                .setLargo(vectorNumber[3])
                .setAncho(vectorNumber[5])
                .setAlto(vectorNumber[7])
                .setRotations(Punto::Build(vectorNumber[2], vectorNumber[4], vectorNumber[6]));

            dataSet[countInstance].itemsByTpe.push_back(itemBin.setId(itemBin.getTipo()));
            for (int numberOfTypedItems = 0; numberOfTypedItems < vectorNumber[1]; numberOfTypedItems++)
            {
                dataSet[countInstance].totalItems.push_back(itemBin.setId(vectorID++));
            }
            if (dataSet[countInstance].totalItems.size() == totalNumberOfItems && countInstance != totalNumberOfInstances - 1)
            {
                if (numberOftypes != dataSet[countInstance].itemsByTpe.size())
                {
                    throw std::invalid_argument("Item types list are not equal as expected.");
                }
                numberOftypes = 0;
                vectorID = 1;
                countInstance++;
            }
            break;
        }
    }
    if (countInstance != dataSet.size() - 1)
    {
        cout << dataSet.size() << " | " << countInstance << "\n ";
        throw invalid_argument("Data set has not equal values");
    }

    return dataSet;
}

vector<vector<int>> getNumberListFile(const string path)
{
    string line;
    ifstream myfile;
    vector<vector<int>> fileByLine;
    myfile.open(path);
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            vector<int> capturedNumbers = getNumbersFromLines(line);
            fileByLine.push_back(capturedNumbers);
        }
        myfile.close();
    }
    else
    {
        cout << "Could not be opened.\n";
    }

    return fileByLine;
}

vector<int> getNumbersFromLines(string line)
{
    vector<int> numbers;
    string number;
    for (auto ch : line)
    {
        if (ch == ' ' || ch == '\n')
        {
            numbers.push_back(stoi(number));
            number = "";
        }
        else
        {
            number += ch;
        }
    }
    numbers.push_back(stoi(number));

    return numbers;
}
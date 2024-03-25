#include "Env.hpp"

std::string getEnv(std::string parameter)
{
    std::string line, word, param;
    std::ifstream myfile;
    int numChar;
    myfile.open(".env");

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            param = "";
            word = "";
            numChar = 0;
            for (auto ch : line)
            {
                numChar++;
                if (numChar == line.size() + 1)
                {
                    if (param == parameter)
                    {
                        myfile.close();
                        return word;
                    }
                    else
                    {
                        numChar = 0;
                        param = "";
                    }
                }
                if (ch != '=')
                {
                    if (param != parameter)
                    {
                        param += ch;
                    }
                    else
                    {
                        word += ch;
                    }
                }
                else if (ch == '=')
                {
                    if (param != parameter)
                    {
                        param = "";
                    }
                }
            }
        }
        myfile.close();
    }
    else
    {
        std::cerr << ".env FileNotFound\n";
    }
    if (word == "")
    {
        std::cerr << ".env parameter not found.\n";
    }

    return word;
}

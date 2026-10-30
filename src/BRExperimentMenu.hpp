#ifndef BR_EXPERIMENT_MENU_H
#define BR_EXPERIMENT_MENU_H
#include "readData/CommandLine.hpp"
#include <istream>
#include <ostream>

struct BRMenuSelection
{
    CommandLineOptions options;
    std::string sessionName = "sesion";
    bool confirmed = false;
};

class BRExperimentMenu
{
  public:
    BRMenuSelection choose(std::istream &, std::ostream &, const CommandLineOptions &defaults) const;
};
#endif

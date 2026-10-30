#ifndef BR_EXPERIMENT_SESSION_H
#define BR_EXPERIMENT_SESSION_H
#include "BRExperimentMenu.hpp"
#include "readData/ExperimentRunner.hpp"

class BRExperimentSession
{
  public:
    // Returns the created session directory, or an empty string on cancellation.
    std::string execute(const BRMenuSelection &) const;
    std::string resume(const CommandLineOptions &) const;
    void writeReport(const std::string &directory, const CommandLineOptions &,
                     const std::vector<ExperimentOutcome> &) const;
};
#endif

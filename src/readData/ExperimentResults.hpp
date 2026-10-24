#ifndef EXPERIMENT_RESULTS_H
#define EXPERIMENT_RESULTS_H
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

enum class ExperimentStatus { Pending, Running, Complete };
struct ExperimentResult
{
    double fitness = 0;
    std::int64_t milliseconds = 0;
    ExperimentStatus status = ExperimentStatus::Pending;
};
using ExperimentResults = std::vector<ExperimentResult>;
// Complete records remain complete at zero fitness or zero elapsed milliseconds.
// Running records are retried on resume. The checkpoint is authoritative; CSVs
// are compatibility exports. Legacy ambiguous (0,0) records are retried once.
bool isExperimentComplete(const ExperimentResult &);
ExperimentResults loadExperimentResults(const std::string &fitnessPath, const std::string &timePath, std::size_t count);
void saveExperimentResults(const ExperimentResults &, const std::string &fitnessPath, const std::string &timePath);
#endif

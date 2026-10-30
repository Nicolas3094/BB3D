#ifndef EXPERIMENT_RESULTS_H
#define EXPERIMENT_RESULTS_H
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

enum class ExperimentStatus
{
    Pending,
    Running,
    Complete
};

struct ExperimentResult
{
    double fitness = 0;
    std::int64_t milliseconds = 0;
    ExperimentStatus status = ExperimentStatus::Pending;
    std::string termination = "completed";
    bool partialEvaluation = false;
};

using ExperimentResults = std::vector<ExperimentResult>;
// Complete records remain complete at zero fitness or zero elapsed milliseconds.
// Running records are retried on resume. The checkpoint is authoritative; CSVs
// are compatibility exports. Legacy ambiguous (0,0) records are retried once.
bool isExperimentComplete(const ExperimentResult &);
std::string experimentFingerprint(const std::string &bytes);
std::string datasetFingerprint(const std::string &path);
std::string createResultSessionDirectory(const std::string &base, const std::string &label);
void saveResultText(const std::string &path, const std::string &text);
ExperimentResults loadExperimentResults(const std::string &fitnessPath, const std::string &timePath,
                                        std::size_t count, const std::string &identity = "");
void saveExperimentResults(const ExperimentResults &, const std::string &fitnessPath,
                           const std::string &timePath, const std::string &identity = "");
#endif

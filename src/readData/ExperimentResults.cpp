#include "ExperimentResults.hpp"
#include "ReadData.hpp"
#include <atomic>
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <ctime>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace
{
void validate(const ExperimentResult &result)
{
    if ((result.termination != "completed" && result.termination != "optimal" &&
         result.termination != "time_limit") ||
        (result.partialEvaluation && result.termination != "time_limit"))
        throw std::invalid_argument("Invalid execution termination metadata.");

    if (result.status == ExperimentStatus::Complete)
    {
        if (!std::isfinite(result.fitness) || result.fitness < 0 || result.fitness > 1 ||
            result.milliseconds < 0)
            throw std::invalid_argument("Completed experiment has invalid fitness or duration.");
    }
    else if (result.status == ExperimentStatus::Running)
    {
        if (result.fitness != -1 || result.milliseconds != -1)
            throw std::invalid_argument("Running experiment must use the -1 marker.");
    }
    else if (result.status != ExperimentStatus::Pending || result.fitness != 0 || result.milliseconds != 0)
        throw std::invalid_argument("Invalid pending experiment or status.");
}

void ensureParents(const std::string &path)
{
    for (size_t i = 0; i < path.size(); ++i)
    {
        if (path[i] != '/' && path[i] != '\\')
            continue;

        const std::string prefix = path.substr(0, i);

        if (prefix.empty() || (prefix.size() == 2 && prefix[1] == ':'))
            continue;
#ifdef _WIN32
        const int result = _mkdir(prefix.c_str());
#else
        const int result = mkdir(prefix.c_str(), 0777);
#endif
        if (result != 0 && errno != EEXIST)
            throw std::runtime_error("Cannot create result directory: " + prefix);
    }
}

void writeAtomic(const std::string &path, const std::string &content)
{
    ensureParents(path);
    static std::atomic<unsigned long long> sequence{0};
#ifdef _WIN32
    const auto process = GetCurrentProcessId();
#else
    const auto process = getpid();
#endif
    const std::string temporary = path + ".tmp." + std::to_string(process) + "." + std::to_string(sequence++);
    try
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);

        if (!output)
            throw std::runtime_error("Cannot create temporary results file: " + path);

        output << content;
        output.flush();

        if (!output)
            throw std::runtime_error("Cannot write results file: " + path);

        output.close();

        if (output.fail())
            throw std::runtime_error("Cannot close results file: " + path);
#ifdef _WIN32
        HANDLE file = CreateFileA(temporary.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Cannot flush results file: " + path);

        const bool flushed = FlushFileBuffers(file) != 0;
        CloseHandle(file);

        if (!flushed)
            throw std::runtime_error("Cannot flush results file: " + path);

        if (!MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot atomically replace results file: " + path);
#else
        const int file = open(temporary.c_str(), O_WRONLY);

        if (file < 0)
            throw std::runtime_error("Cannot flush results file: " + path);

        const bool flushed = fsync(file) == 0;
        close(file);

        if (!flushed || std::rename(temporary.c_str(), path.c_str()) != 0)
            throw std::runtime_error("Cannot atomically replace results file: " + path);
#endif
    }
    catch (...)
    {
        std::remove(temporary.c_str());
        throw;
    }
}

void validatePaths(const std::string &fitnessPath, const std::string &timePath)
{
    if (fitnessPath.empty() || timePath.empty() || fitnessPath == timePath ||
        fitnessPath + ".checkpoint" == timePath)
        throw std::invalid_argument("Fitness, time and checkpoint files must be distinct.");
}
}

bool isExperimentComplete(const ExperimentResult &result)
{
    return result.status == ExperimentStatus::Complete && std::isfinite(result.fitness) &&
           result.fitness >= 0 && result.fitness <= 1 && result.milliseconds >= 0;
}

void saveResultText(const std::string &path, const std::string &text)
{
    writeAtomic(path, text);
}

std::string createResultSessionDirectory(const std::string &base, const std::string &label)
{
    if (base.empty() || label.empty() ||
        label.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") !=
            std::string::npos)
        throw std::invalid_argument(
            "Session label must contain only letters, digits, underscores or hyphens.");
    const auto now = std::time(nullptr);
    const auto local = std::localtime(&now);

    if (!local)
        throw std::runtime_error("Cannot determine session date.");

    std::ostringstream stamp;
    stamp << std::put_time(local, "%Y%m%d-%H%M%S");
    ensureParents(base + "/.session");

    for (unsigned long long suffix = 0;; ++suffix)
    {
        const auto path =
            base + "/" + label + "-" + stamp.str() + (suffix ? "-" + std::to_string(suffix) : "");
#ifdef _WIN32
        const int result = _mkdir(path.c_str());
#else
        const int result = mkdir(path.c_str(), 0777);
#endif
        if (result == 0)
            return path;

        if (errno != EEXIST)
            throw std::runtime_error("Cannot create session directory: " + path);
    }
}

std::string experimentFingerprint(const std::string &bytes)
{
    std::uint64_t hash = 14695981039346656037ULL;

    for (unsigned char byte : bytes)
    {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << hash;

    return out.str();
}

std::string datasetFingerprint(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("Cannot fingerprint dataset: " + path);

    std::ostringstream bytes;
    bytes << file.rdbuf();

    if (file.bad())
        throw std::runtime_error("Cannot fingerprint dataset: " + path);

    return experimentFingerprint(bytes.str());
}

ExperimentResults loadExperimentResults(const std::string &fitnessPath, const std::string &timePath,
                                        size_t count, const std::string &identity)
{
    validatePaths(fitnessPath, timePath);
    errno = 0;
    std::ifstream checkpoint(fitnessPath + ".checkpoint");

    if (checkpoint.is_open())
    {
        std::string line;

        if (!std::getline(checkpoint, line))
            throw std::invalid_argument("Missing checkpoint header.");

        const bool version3 = line == "BB3D_RESULTS_V3";

        if (identity.empty() ? line != "BB3D_RESULTS_V1" : (!version3 && line != "BB3D_RESULTS_V2"))
            throw std::invalid_argument("Invalid experiment checkpoint header: " + fitnessPath);

        if (!identity.empty() && (!std::getline(checkpoint, line) || line != identity))
            throw std::invalid_argument(
                "Checkpoint dataset, seed or configuration differs from this experiment.");
        ExperimentResults results;

        while (std::getline(checkpoint, line))
        {
            if (std::count(line.begin(), line.end(), ',') != (version3 ? 4 : 2))
                throw std::invalid_argument("Invalid experiment checkpoint columns: " + fitnessPath);

            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream input(line);
            input.imbue(std::locale::classic());
            int status;
            ExperimentResult result;
            std::string extra;

            if (!(input >> status >> result.fitness >> result.milliseconds) || status < 0 || status > 2)
                throw std::invalid_argument("Invalid experiment checkpoint row: " + fitnessPath);

            if (version3)
            {
                int partial;

                if (!(input >> result.termination >> partial) || (partial != 0 && partial != 1))
                    throw std::invalid_argument("Invalid execution metadata in checkpoint.");

                result.partialEvaluation = partial != 0;
            }
            if (input >> extra)
                throw std::invalid_argument("Extra experiment checkpoint fields.");

            result.status = static_cast<ExperimentStatus>(status);
            validate(result);
            results.push_back(result);
        }
        if (checkpoint.bad())
            throw std::runtime_error("Cannot read experiment checkpoint: " + fitnessPath);

        if (results.size() != count)
            throw std::invalid_argument("Checkpoint size differs from the dataset instance count.");

        return results;
    }
    if (errno != ENOENT)
        throw std::runtime_error("Cannot open experiment checkpoint: " + fitnessPath);

    const auto fitness = getFloatNumberListFile(fitnessPath);
    const auto times = getIntegerNumberListFile(timePath);

    if (!identity.empty() && (!fitness.empty() || !times.empty()))
        throw std::invalid_argument("Legacy results have no experiment identity; use a fresh result path.");

    if (fitness.size() > count || times.size() > count)
        throw std::invalid_argument("Legacy results exceed the dataset instance count.");

    for (double value : fitness)
        if (value != -1 && (value < 0 || value > 1))
            throw std::invalid_argument("Invalid legacy fitness value.");

    for (auto value : times)
        if (value < -1)
            throw std::invalid_argument("Invalid legacy duration value.");

    ExperimentResults results(count);

    for (size_t i = 0; i < count && i < fitness.size() && i < times.size(); ++i)
    {
        if (fitness[i] == -1 || times[i] == -1)
            results[i] = {-1, -1, ExperimentStatus::Running};
        else if (fitness[i] > 0 || times[i] > 0)
            results[i] = {fitness[i], times[i], ExperimentStatus::Complete};
        // Legacy (0,0) has no explicit completion flag: retry once, then persist status.
    }
    return results;
}

void saveExperimentResults(const ExperimentResults &results, const std::string &fitnessPath,
                           const std::string &timePath, const std::string &identity)
{
    if (identity.find_first_of("\r\n") != std::string::npos)
        throw std::invalid_argument("Experiment identity must be one line.");

    validatePaths(fitnessPath, timePath);

    for (const auto &result : results)
        validate(result);
    if (identity.empty())
        for (const auto &result : results)
            if (result.termination != "completed" || result.partialEvaluation)
                throw std::invalid_argument("Execution metadata requires a bound experiment identity.");

    std::ostringstream checkpoint, fitness, times;
    checkpoint.imbue(std::locale::classic());
    fitness.imbue(std::locale::classic());
    times.imbue(std::locale::classic());
    checkpoint << (identity.empty() ? "BB3D_RESULTS_V1\n" : "BB3D_RESULTS_V3\n" + identity + "\n")
               << std::setprecision(std::numeric_limits<double>::max_digits10);
    fitness << std::setprecision(std::numeric_limits<double>::max_digits10);

    for (const auto &result : results)
    {
        checkpoint << static_cast<int>(result.status) << ',' << result.fitness << ',' << result.milliseconds;

        if (!identity.empty())
            checkpoint << ',' << result.termination << ',' << result.partialEvaluation;
        checkpoint << '\n';
        fitness << result.fitness << '\n';
        times << result.milliseconds << '\n';
    }
    // The single authoritative checkpoint is committed before compatibility exports.
    // A crash between the two CSV exports cannot mix fitness and duration on resume.
    writeAtomic(fitnessPath + ".checkpoint", checkpoint.str());
    writeAtomic(fitnessPath, fitness.str());
    writeAtomic(timePath, times.str());
}

#ifndef EXPERIMENT_EXECUTION_H
#define EXPERIMENT_EXECUTION_H
#include "../readData/DataSetBPP.hpp"
#include "../genetic/Individuo.hpp"
#include <chrono>
#include <functional>
#include <utility>
struct ExperimentConfiguration;
enum class ExecutionMode {Iterations,ThirtySeconds};
struct ExecutionResult {
    Poblacion population;
    std::int64_t milliseconds=0;
    bool timeLimitReached=false,partialEvaluation=false;
    std::string termination="completed";
};
class ExperimentExecution {
public:
    virtual ~ExperimentExecution()=default;
    virtual ExecutionResult execute(const ExperimentConfiguration &,DatasetBinBacking) const=0;
};
class IterationExecution final : public ExperimentExecution {
public:
    ExecutionResult execute(const ExperimentConfiguration &,DatasetBinBacking) const override;
};
class ThirtySecondExecution final : public ExperimentExecution {
public:
    using Now=std::function<std::chrono::steady_clock::time_point()>;
    explicit ThirtySecondExecution(Now now=std::chrono::steady_clock::now):now(std::move(now)){}
    ExecutionResult execute(const ExperimentConfiguration &,DatasetBinBacking) const override;
    static constexpr std::int64_t limitMilliseconds=30000;
private:
    Now now;
};
#endif

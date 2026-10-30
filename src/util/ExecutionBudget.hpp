#ifndef EXECUTION_BUDGET_H
#define EXECUTION_BUDGET_H
#include "ExecutionCheckpoint.hpp"
#include "../genetic/Individuo.hpp"
#include <chrono>
#include <functional>

class ExecutionBudget
{
  public:
    using Clock = std::chrono::steady_clock;
    using Now = std::function<Clock::time_point()>;
    ExecutionBudget(std::chrono::milliseconds limit, Individuo fallback, Now now = Clock::now);
    ~ExecutionBudget();
    ExecutionBudget(const ExecutionBudget &) = delete;
    ExecutionBudget &operator=(const ExecutionBudget &) = delete;
    void checkpoint();
    void observe(const Individuo &, bool partial);

    const Individuo &best() const
    {
        return incumbent;
    }

    bool partial() const
    {
        return incumbentPartial;
    }

  private:
    ExecutionBudget *previous;
    Now now;
    Clock::time_point deadline;
    Individuo incumbent;
    bool incumbentPartial = true;
};

void recordExecutionCandidate(const Individuo &, bool partial = false);
#endif

#include "ExecutionBudget.hpp"
#include <cmath>
#include <utility>

namespace
{
thread_local ExecutionBudget *active = nullptr;
}

ExecutionBudget::ExecutionBudget(std::chrono::milliseconds limit, Individuo fallback, Now clock)
    : previous(active), now(std::move(clock)), incumbent(std::move(fallback))
{
    if (limit.count() <= 0 || !now)
        throw std::invalid_argument("Execution budget requires a positive duration and a clock.");

    deadline = now() + limit;
    active = this;
}

ExecutionBudget::~ExecutionBudget()
{
    active = previous;
}

void ExecutionBudget::checkpoint()
{
    if (now() >= deadline)
        throw ExecutionTimeLimit();
}

void ExecutionBudget::observe(const Individuo &candidate, bool partial)
{
    const auto fitness = candidate.getFitness();

    if (!std::isfinite(fitness) || fitness < 0 || fitness > 1)
        return;

    if (fitness > incumbent.getFitness() ||
        (fitness == incumbent.getFitness() && incumbentPartial && !partial))
    {
        incumbent = candidate;
        incumbentPartial = partial;
    }
}

void checkExecutionDeadline()
{
    if (active)
        active->checkpoint();
}

void recordExecutionCandidate(const Individuo &candidate, bool partial)
{
    if (active)
        active->observe(candidate, partial);
}

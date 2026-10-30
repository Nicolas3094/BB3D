#ifndef EXECUTION_CHECKPOINT_H
#define EXECUTION_CHECKPOINT_H
#include <exception>

class ExecutionTimeLimit : public std::exception
{
  public:
    const char *what() const noexcept override
    {
        return "Execution time limit reached.";
    }
};

void checkExecutionDeadline();
#endif

#include <exception>

#include "sclp/Core.hpp" // IWYU pragma: keep

using namespace sclp;

int main(const int argc, const char * const argv[]) {
    try {
        auto problem = std::make_shared<Problem>(argc, argv);
        // parse cli
        if (problem->cli_parameters.io.help) {
            problem->cli_parameters.help_message();
            return 0;
        }
        problem->initialize();
        problem->solve();

        auto logs = Logs(problem);
        logs.run();

        return 0;
    } catch (std::exception const & error) {
        fmt::println("{}", error.what());
        return 1;
    }
}

#ifndef SCLP_PROBLEM_CLI_PARAMETERS_HPP_
#define SCLP_PROBLEM_CLI_PARAMETERS_HPP_

#include <memory>
#include <stdexcept>
#include <string>

#include <cxxopts.hpp>

#include "sclp/Types.hpp" // IWYU pragma: keep

#include "sclp/Allocation/GreedySearch.hpp"

namespace sclp {

class CliParameters {
public:
    struct IO {
        bool help;
        bool irace;
        std::string instance_file;
        std::string output_file;

        auto to_json() const -> Json {
            auto obj = Json();
            obj["help"] = help;
            obj["irace"] = irace;
            obj["instance_file"] = instance_file;
            obj["output_file"] = output_file;
            return obj;
        }
    };

    struct General {
        double maximum_running_time_seconds;

        auto to_json() const -> Json {
            auto obj = Json();
            obj["maximum_running_time_seconds"] = maximum_running_time_seconds;
            return obj;
        }
    };

    struct FitnessFunction {
        double alpha;
        double beta;
        double gamma;
        double delta;
        double alpha_2;
        double beta_2;
        double gamma_2;
        double delta_2;
        double configuration_switch_threshold;

        auto to_json() const -> Json {
            auto obj = Json();
            obj["alpha"] = alpha;
            obj["beta"] = beta;
            obj["gamma"] = gamma;
            obj["delta"] = delta;
            obj["alpha_2"] = alpha_2;
            obj["beta_2"] = beta_2;
            obj["gamma_2"] = gamma_2;
            obj["delta_2"] = delta_2;
            obj["configuration_switch_threshold"] = configuration_switch_threshold;
            return obj;
        }
    };

    struct Greedy {
        GreedySearch::SkipType skip_type;

        struct SkipTypeParser {
            static auto parse(const std::string value_str) -> GreedySearch::SkipType {
                if (value_str == "none") {
                    return GreedySearch::SkipType::NONE;
                }
                if (value_str == "loose") {
                    return GreedySearch::SkipType::LOOSE;
                }
                if (value_str == "strict") {
                    return GreedySearch::SkipType::STRICT;
                }
                throw std::invalid_argument("Invalid greedy search skip type " + value_str);
            }

            static auto parse(const GreedySearch::SkipType skip_type) -> std::string {
                switch (skip_type) {
                case GreedySearch::SkipType::NONE:
                    return "none";
                case GreedySearch::SkipType::LOOSE:
                    return "loose";
                case GreedySearch::SkipType::STRICT:
                    return "strict";
                default:
                    throw std::invalid_argument("Invalid greedy search skip type " + std::to_string(static_cast<int>(skip_type)));
                }
            }
        };

        auto to_json() const -> Json {
            auto obj = Json();
            obj["skip_type"] = SkipTypeParser::parse(skip_type);
            return obj;
        }
    };

    struct TreeSearch {
        int initial_branching_factor;
        int maximum_branching_factor;
        double block_minimum_volume_ratio;
        int maximum_number_of_blocks;

        auto to_json() const -> Json {
            auto obj = Json();
            obj["initial_branching_factor"] = initial_branching_factor;
            obj["maximum_branching_factor"] = maximum_branching_factor;
            obj["block_minimum_volume_ratio"] = block_minimum_volume_ratio;
            obj["maximum_number_of_blocks"] = maximum_number_of_blocks;
            return obj;
        }
    };

    IO io;
    General general;
    TreeSearch tree_search;
    Greedy greedy;
    FitnessFunction fitness_function;

    CliParameters(const int argc, const char * const argv[]) {
        options = std::make_unique<cxxopts::Options>(cxxopts::Options(program_name, program_description));

        // IO
        options->add_options()("h,help", "Print this help message", cxxopts::value<bool>()->implicit_value("true")->default_value("false"));
        options->add_options()("irace", "set output for irace", cxxopts::value<bool>()->implicit_value("true")->default_value("false"));
        options->add_options()("instance-file", "path to the instance file", cxxopts::value<std::string>()->default_value("./instance.json"));
        options->add_options()("output-file", "path to the output file, where the packing will be saved", cxxopts::value<std::string>()->default_value("./output.json"));
        // General
        options->add_options()("maximum_running_time_seconds", "Maximum running time, in seconds.", cxxopts::value<double>()->default_value("5"));
        // FitnessFunction
        options->add_options()("alpha", "control the influence of the contact area in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("4.0"));
        options->add_options()("beta", "control the influence of the volume loss in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("1.0"));
        options->add_options()("gamma", "control the influence of the number of items in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("0.2"));
        options->add_options()("delta", "the relative tolerance to use when computing contact between blocks in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("0.04"));
        options->add_options()("alpha_2", "control the influence of the contact area in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("4.0"));
        options->add_options()("beta_2", "control the influence of the volume loss in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("1.0"));
        options->add_options()("gamma_2", "control the influence of the number of items in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("0.2"));
        options->add_options()("delta_2", "the relative tolerance to use when computing contact between blocks in the fitness function, a real value greater or equals 0.0", cxxopts::value<double>()->default_value("0.04"));
        options->add_options()("configuration_switch_threshold", "the threshold to use to switch between the two configurations; when the fill rate of the large object is larger than this threshold, the fitness computer will use the second configuration; a real value in the interval [0.0, 1.0] (use a value greater than 1.0 to disable the second configuration)", cxxopts::value<double>()->default_value("1.01"));
        // Greedy
        options->add_options()("skip_type", "skip technique to use in the greedy search, one of {none, loose, strict}", cxxopts::value<std::string>()->default_value("none"));
        // TreeSearch
        options->add_options()("initial_branching_factor", "initial branching factor value, an integer value greater or equals to 2", cxxopts::value<int>()->default_value("4"));
        options->add_options()("maximum_branching_factor", "maximum branching factor value, an integer value greater or equals to 2", cxxopts::value<int>()->default_value("10000"));
        options->add_options()("block_minimum_volume_ratio", "minimum filled volume ratio required for a guillotine block, a value in the interval ]0.0, 1.0]. Use -1.0 to set automatically based on the number of items of the input (number of items < 30 ? 1.00 : 0.98)", cxxopts::value<double>()->default_value("-1.0"));
        options->add_options()("maximum_number_of_blocks", "maximum number of blocks to be generated using the guillotine blocks method, an integer value greater or equals to 1", cxxopts::value<int>()->default_value("10000"));

        options->parse_positional({"instance-file"});
        options->positional_help("INSTANCE-FILE");

        parse_result = options->parse(argc, argv);

        // IO
        io.help = parse_result["help"].as<bool>();
        io.irace = parse_result["irace"].as<bool>();
        io.instance_file = parse_result["instance-file"].as<std::string>();
        io.output_file = parse_result["output-file"].as<std::string>();
        // General
        general.maximum_running_time_seconds = parse_result["maximum_running_time_seconds"].as<double>();
        // FitnessFunction
        fitness_function.alpha = parse_result["alpha"].as<double>();
        fitness_function.beta = parse_result["beta"].as<double>();
        fitness_function.gamma = parse_result["gamma"].as<double>();
        fitness_function.delta = parse_result["delta"].as<double>();
        fitness_function.alpha_2 = parse_result["alpha_2"].as<double>();
        fitness_function.beta_2 = parse_result["beta_2"].as<double>();
        fitness_function.gamma_2 = parse_result["gamma_2"].as<double>();
        fitness_function.delta_2 = parse_result["delta_2"].as<double>();
        fitness_function.configuration_switch_threshold = parse_result["configuration_switch_threshold"].as<double>();
        // Greedy
        greedy.skip_type = Greedy::SkipTypeParser::parse(parse_result["skip_type"].as<std::string>());
        // TreeSearch
        tree_search.initial_branching_factor = parse_result["initial_branching_factor"].as<int>();
        tree_search.maximum_branching_factor = parse_result["maximum_branching_factor"].as<int>();
        tree_search.block_minimum_volume_ratio = parse_result["block_minimum_volume_ratio"].as<double>();
        tree_search.maximum_number_of_blocks = parse_result["maximum_number_of_blocks"].as<int>();
    }

    auto help_message() const -> void {
        fmt::println("{}", options->help());
        return;
    }

    auto to_json() const -> Json {
        auto obj = Json();
        obj["io"] = io.to_json();
        obj["general"] = general.to_json();
        obj["tree_search"] = tree_search.to_json();
        obj["greedy"] = greedy.to_json();
        obj["fitness_function"] = fitness_function.to_json();
        return obj;
    }

private:
    std::string const program_name = "allocate";
    std::string const program_description = "solver for the Single Large Object Placement Problem (SCLP)";
    std::unique_ptr<cxxopts::Options> options;
    cxxopts::ParseResult parse_result;
};

} // namespace sclp

#endif

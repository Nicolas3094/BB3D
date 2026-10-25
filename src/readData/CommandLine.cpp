#include "CommandLine.hpp"
#include <limits>
#include <unordered_set>
#include <cmath>

namespace {
int integer(const std::string &value) {
    const auto number=parseExperimentSeed(value);
    if(number>static_cast<std::uint64_t>(std::numeric_limits<int>::max())) throw std::invalid_argument("Integer option is too large.");
    return static_cast<int>(number);
}
double probability(const std::string &text) {
    std::istringstream input(text);input.imbue(std::locale::classic());double p;std::string extra;
    if(!(input>>p) || input>>extra || !std::isfinite(p) || p<0 || p>1) throw std::invalid_argument("Probability must be in [0,1].");
    return p;
}
}
CommandLineOptions parseCommandLine(const std::vector<std::string> &arguments) {
    CommandLineOptions o;if(arguments.empty()) {o.interactive=true;return o;}
    std::unordered_set<std::string> seen;
    for(size_t i=0;i<arguments.size();++i) {
        const auto &flag=arguments[i];
        if(!seen.insert(flag).second) throw std::invalid_argument("Repeated option: "+flag);
        if(flag=="--help") {o.help=true;continue;}
        if(flag=="--menu") {o.interactive=true;continue;}
        if(flag=="--report") {o.sessionReport=true;continue;}
        if(flag=="--batch") {o.algorithm="all";o.dataset="all";o.rotation="all";o.mutation="all";o.replacement="both";continue;}
        if(i+1>=arguments.size()) throw std::invalid_argument("Option requires a value: "+flag);
        const auto &value=arguments[++i];
        if(flag=="--algorithm") o.algorithm=value;
        else if(flag=="--dataset") o.dataset=value;
        else if(flag=="--root") o.root=value;
        else if(flag=="--output") o.output=value;
        else if(flag=="--seed") o.seed=parseExperimentSeed(value);
        else if(flag=="--execution") {
            if(value=="iterations")o.execution=ExecutionMode::Iterations;
            else if(value=="30s")o.execution=ExecutionMode::ThirtySeconds;
            else throw std::invalid_argument("Execution must be iterations or 30s.");
        }
        else if(flag=="--rotation") o.rotation=value;
        else if(flag=="--mutation") o.mutation=value;
        else if(flag=="--replacement") o.replacement=value;
        else if(flag=="--population") {o.population=integer(value);if(o.population==0) throw std::invalid_argument("Population must be positive.");}
        else if(flag=="--iterations") o.iterations=integer(value);
        else if(flag=="--rounds") {o.rounds=integer(value);if(o.rounds==0) throw std::invalid_argument("Rounds must be positive.");}
        else if(flag=="--instances") o.instances=integer(value);
        else if(flag=="--first-instance") {o.firstInstance=integer(value);if(o.firstInstance==0) throw std::invalid_argument("First instance must be positive.");}
        else if(flag=="--sites") o.sites=integer(value);
        else if(flag=="--elite-sites") o.eliteSites=integer(value);
        else if(flag=="--elite-bees") o.eliteBees=integer(value);
        else if(flag=="--non-elite-bees") o.nonEliteBees=integer(value);
        else if(flag=="--movements") o.movements=integer(value);
        else if(flag=="--patience") o.patience=integer(value);
        else if(flag=="--selection") o.selection=probability(value);
        else if(flag=="--crossover") o.crossover=probability(value);
        else if(flag=="--diversity") o.diversity=probability(value);
        else if(flag=="--mutation-probability") o.mutationProbability=value=="adaptive"?-1:probability(value);
        else if(flag=="--rotation-mutation") o.rotationMutation=probability(value);
        else if(flag=="--cache-entries") o.cacheEntries=integer(value);
        else if(flag=="--cache-mib") {
            auto size=parseExperimentSeed(value);
            if(size>std::numeric_limits<std::size_t>::max()/(1024*1024)) throw std::invalid_argument("Cache budget is too large.");
            o.cacheBytes=static_cast<std::size_t>(size)*1024*1024;
        }
        else if(flag=="--candidates") {
            if(value=="legacy") o.candidates=DBLFCandidatePolicy::ThreePoints;
            else if(value=="projected") o.candidates=DBLFCandidatePolicy::Projected;
            else if(value=="best") o.candidates=DBLFCandidatePolicy::BestOfBoth;
            else throw std::invalid_argument("Unknown candidate policy.");
        } else throw std::invalid_argument("Unknown option: "+flag);
    }
    if(o.algorithm!="GA" && o.algorithm!="ABC" && o.algorithm!="FFA" && o.algorithm!="all") throw std::invalid_argument("Algorithm must be GA, ABC, FFA or all.");
    if(o.rotation!="0" && o.rotation!="2" && o.rotation!="6" && o.rotation!="all") throw std::invalid_argument("Rotation must be 0, 2, 6 or all.");
    if(o.mutation!="inverse" && o.mutation!="c1" && o.mutation!="c2" && o.mutation!="all") throw std::invalid_argument("Mutation must be inverse, c1, c2 or all.");
    if(o.replacement!="true" && o.replacement!="false" && o.replacement!="both") throw std::invalid_argument("Replacement must be true, false or both.");
    if(o.dataset.empty()) throw std::invalid_argument("Dataset cannot be empty.");
    return o;
}
ExperimentConfiguration CommandLineOptions::configuration(const std::string &name,ROTATION_WAY r,MutationType m,bool replace) const {
    auto c=makeExperimentConfiguration(name,r,m,replace);c.candidates=candidates;c.iterations=iterations;
    c.execution=execution;
    if(population!=-1) c.individuals=population;
    c.sites=sites==-1?std::min(c.sites,c.individuals):sites;
    c.eliteSites=eliteSites==-1?std::min(c.eliteSites,c.sites):eliteSites;
    c.eliteBees=eliteBees;c.nonEliteBees=nonEliteBees;c.movements=movements;c.patience=patience;
    c.selection=selection;c.crossover=crossover;c.diversity=diversity;c.cacheEntries=cacheEntries;c.cacheBytes=cacheBytes;
    if(mutationProbability!=-1) {c.mutationProbability=mutationProbability;c.adaptive=false;}
    if(rotationMutation!=-1) c.rotationMutation=rotationMutation;
    if(name=="GA" && c.individuals<2) throw std::invalid_argument("GA requires at least two individuals.");
    if(name=="ABC" && (c.sites>c.individuals || c.eliteSites>c.sites)) throw std::invalid_argument("ABC requires elites <= sites <= population.");
    return c;
}
std::string commandLineHelp() {
    return "BB3D experiments (no arguments: interactive BR menu; --menu also opens it)\n"
      "--algorithm GA|ABC|FFA|all  --dataset P1A2..P5A2|BR0..BR18|BR (all BR)|all|FILE\n"
      "--root DIRECTORY (otherwise LOCAL_PATH in .env)  --output DIRECTORY\n"
      "--population N --iterations N --rounds N --instances N (0: all) --first-instance N --seed N\n"
      "--execution iterations|30s (30 seconds per problem/algorithm/round; ignores iteration/stagnation limits)\n"
      "--rotation 0|2|6|all --mutation inverse|c1|c2|all --replacement true|false|both\n"
      "--selection P --crossover P --mutation-probability adaptive|P --rotation-mutation P\n"
      "--patience N (0: disable stagnation stop) --diversity P --movements N\n"
      "--sites N --elite-sites N --elite-bees N --non-elite-bees N\n"
      "--candidates legacy|projected|best --cache-entries N (default 0: disabled)\n"
      "--cache-mib N (payload budget; 0 disables cache)\n"
      "--report regenerates a BR session report in the explicit --output directory\n"
      "--batch (all algorithms, datasets, rotations, mutations and replacement modes)\n";
}

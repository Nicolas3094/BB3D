#include "../src/readData/ExperimentConfiguration.hpp"
#include "../src/readData/ExperimentResults.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/util/ExecutionBudget.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include "../src/binPacking/ItemBinOperators.hpp"
#include <sstream>

void require(bool ok,const char *message){if(!ok)throw std::runtime_error(message);}
template<class F> void rejects(F f){bool caught=false;try{f();}catch(const std::exception &){caught=true;}require(caught,"Invalid timed execution accepted");}
DatasetBinBacking problem(){
    DatasetBinBacking data{};data.bin=Bin::Build().setLargo(4).setAncho(4).setAlto(4);
    for(int id=1;id<=6;++id)data.totalItems.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(1).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1)));
    return data;
}
Individuo candidate(){return Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1,2,3,4,5,6}).setDGenome({0,0,0,0,0,0})).setFitness(0);}
void budgets(){
    auto now=ExecutionBudget::Clock::time_point{};auto fallback=candidate();fallback.solution="|";
    {
        ExecutionBudget budget(std::chrono::seconds(30),fallback,[&]{return now;});
        now+=std::chrono::milliseconds(29999);checkExecutionDeadline();now+=std::chrono::milliseconds(1);
        rejects([]{checkExecutionDeadline();});
        auto best=fallback;best.setFitness(0.5);best.solution="full";recordExecutionCandidate(best);
        best.setFitness(0.75);best.solution="partial";recordExecutionCandidate(best,true);
        require(budget.best().getFitness()==0.75 && budget.partial(),"Better partial packing was discarded");
        recordExecutionCandidate(best,false);require(!budget.partial(),"Complete evaluation did not win a tie");
        best.setFitness(0.25);recordExecutionCandidate(best);require(budget.best().getFitness()==0.75,"Incumbent regressed");
    }
    checkExecutionDeadline();
    rejects([&]{ExecutionBudget invalid(std::chrono::milliseconds(0),fallback);});
}
void strategies(){
    static_assert(ThirtySecondExecution::limitMilliseconds==30000,"Production limit is not thirty seconds");
    for(const std::string algorithm:{"GA","ABC","FFA"})for(bool replacement:{false,true})for(bool cached:{false,true}) {
        auto config=makeExperimentConfiguration(algorithm,ROTATION_WAY::ZERO_WAY,MutationType::C2,replacement);
        config.individuals=4;config.iterations=0;config.sites=2;config.eliteSites=1;config.eliteBees=1;config.nonEliteBees=1;config.movements=1;config.cacheEntries=cached?8:0;
        seedRandomNumbers(123);auto normal=IterationExecution().execute(config,problem());
        require(normal.population.size()==4 && !normal.timeLimitReached && !normal.partialEvaluation,"Iteration strategy changed behavior");
        size_t ticks=0;
        ThirtySecondExecution timed([&]{return ExecutionBudget::Clock::time_point{}+std::chrono::seconds(ticks++);});
        seedRandomNumbers(123);auto result=timed.execute(config,problem());
        require(result.timeLimitReached && result.termination=="time_limit" && result.population.size()==1,"Timed strategy did not stop or return an incumbent");
        require(result.population[0].getFitness()>=0 && result.population[0].getFitness()<=6.0/64 && result.population[0].getGenome().getGenome().size()==6,"Timed strategy returned an invalid result");
        checkExecutionDeadline();
    }
    auto config=makeExperimentConfiguration("GA",ROTATION_WAY::ZERO_WAY,MutationType::INVERSE_MUTATION,true);config.individuals=2;
    auto perfect=problem();perfect.totalItems.resize(1);perfect.bin.setLargo(1).setAncho(1).setAlto(1);
    auto optimal=ThirtySecondExecution([]{return ExecutionBudget::Clock::time_point{};}).execute(config,perfect);
    require(!optimal.timeLimitReached && optimal.termination=="optimal" && optimal.population[0].getFitness()==1,"Optimal solution did not finish before the limit");
}
void partialPackings(){
    bool sawPartial=false;
    for(auto policy:{DBLFCandidatePolicy::ThreePoints,DBLFCandidatePolicy::Projected,DBLFCandidatePolicy::BestOfBoth})
    for(size_t cutoff:{3,10,30,60,120,300}) {
        auto data=problem();data.bin.setRotationWay(ROTATION_WAY::ZERO_WAY).setCandidatePolicy(policy);
        auto ind=candidate();auto fallback=ind;fallback.solution="|";Individuo best;bool interrupted=false,partial=false;size_t ticks=0;
        {
            ExecutionBudget budget(std::chrono::seconds(30),fallback,[&]{return ExecutionBudget::Clock::time_point{}+(++ticks>cutoff?std::chrono::seconds(31):std::chrono::seconds(0));});
            try{evaluateFitnessDecodificated(ind,data.totalItems,data.bin);}catch(const ExecutionTimeLimit &){interrupted=true;}
            best=budget.best();partial=budget.partial();
        }
        if(interrupted && data.bin.getNumberOfLoadedItems()>0 && data.bin.getNumberOfLoadedItems()<6)sawPartial=true;
        require(best.getFitness()==static_cast<double>(data.bin.getLoadedVolume())/64,"Deadline lost a valid partial packing");
        if(interrupted)require(partial,"Interrupted decoded evaluation was marked complete");
        auto verified=problem().bin;
        for(const auto &item:data.bin.getLoadedItems()) {
            require(canBePlaced(verified,item.getPunto()+item.getCurrentDimension()) && !isOverlapped(item.getPunto(),item,verified),"Deadline returned overlapping/out-of-bounds boxes");
            verified.addItem(item);
        }
    }
    require(sawPartial,"Tests never interrupted an evaluation after placing boxes");
}
void metadata(){
    auto options=parseCommandLine({"--execution","30s"});require(options.execution==ExecutionMode::ThirtySeconds,"CLI lost execution mode");
    rejects([]{parseCommandLine({"--execution","31s"});});
    auto config=makeExperimentConfiguration("GA",ROTATION_WAY::ZERO_WAY,MutationType::C2,true);
    auto old=config.identity("hash",123);config.execution=ExecutionMode::ThirtySeconds;
    require(config.identity("hash",123)!=old,"Timed checkpoint identity matches iteration mode");
    const std::string fitness="build/timed-tests/fitness.csv",times="build/timed-tests/time.csv";
    saveExperimentResults({{0.5,30000,ExperimentStatus::Complete,"time_limit",true}},fitness,times,"timed-test");
    const auto loaded=loadExperimentResults(fitness,times,1,"timed-test");
    require(loaded[0].termination=="time_limit" && loaded[0].partialEvaluation && isExperimentComplete(loaded[0]),"Timed result metadata did not survive resume");
    saveResultText(fitness+".checkpoint","BB3D_RESULTS_V2\ntimed-test\n2,0.5,10\n");
    require(loadExperimentResults(fitness,times,1,"timed-test")[0].termination=="completed","Old V2 checkpoints are no longer readable");
    saveResultText(fitness+".checkpoint","BB3D_RESULTS_V3\ntimed-test\n2,0.5,10,time_limit,2\n");
    rejects([&]{loadExperimentResults(fitness,times,1,"timed-test");});
}
int main(){
    std::ostringstream diagnostics;auto *previous=std::cout.rdbuf(diagnostics.rdbuf());
    try{budgets();strategies();partialPackings();metadata();std::cout.rdbuf(previous);std::cout<<"PASS: execution strategies, exact deadline, partial packings, all algorithms/modes, CLI and V2/V3 resume\n";return 0;}
    catch(const std::exception &error){std::cout.rdbuf(previous);std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}

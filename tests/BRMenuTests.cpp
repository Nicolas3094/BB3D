#include "../src/BRExperimentMenu.hpp"
#include "../src/BRExperimentSession.hpp"
#include "../src/readData/ExperimentResults.hpp"
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

void require(bool condition,const char *message){if(!condition)throw std::runtime_error(message);}
template<class F> void rejects(F f){bool caught=false;try{f();}catch(const std::exception &){caught=true;}require(caught,"Invalid session was accepted");}
std::string read(const std::string &path){std::ifstream file(path);require(file.good(),"Missing report file");std::ostringstream result;result<<file.rdbuf();return result.str();}
std::string answers(){return "0\n1\n2\n4\n2\n3\n3\n1\n4\n1\n0\n2\n123\nunit\n0\n1\n";}
void menuTests(){
    CommandLineOptions defaults;defaults.root=".";defaults.output="build/br-menu-tests";
    std::istringstream articleInput("15\n1\n1\n5\n1\narticulo\n1\n");std::ostringstream articleOutput;
    const auto article=BRExperimentMenu().choose(articleInput,articleOutput,defaults);
    require(article.confirmed && article.options.algorithm=="SCLP" && article.options.dataset=="BR15" && article.options.execution==ExecutionMode::ThirtySeconds,"Article algorithm is missing from menu");
    require(articleOutput.str().find("30 segundos CPU")!=std::string::npos && articleOutput.str().find("Metodo de mutacion")==std::string::npos,"Article menu requested unrelated genetic settings");
    std::istringstream articleCancel("15\n1\n1\n5\n1\narticulo\n0\n");std::ostringstream articleCancelled;
    require(!BRExperimentMenu().choose(articleCancel,articleCancelled,defaults).confirmed,"Article cancellation started execution");
    std::istringstream input(answers());std::ostringstream output;
    auto selected=BRExperimentMenu().choose(input,output,defaults);const auto &o=selected.options;
    require(selected.confirmed && o.dataset=="BR0" && o.firstInstance==2 && o.instances==1,"Menu lost BR/problem selection");
    require(o.algorithm=="all" && o.mutation=="c1" && o.rotation=="6" && o.replacement=="both","Menu lost method selection");
    require(o.population==4 && o.iterations==0 && o.rounds==2 && o.seed==123,"Menu lost search parameters");
    auto contactAnswers=answers();const auto contactPosition=contactAnswers.find("3\n1\n4\n1\n0\n");
    require(contactPosition!=std::string::npos,"Contact menu fixture mismatch");
    contactAnswers.replace(contactPosition,10,"3\n4\n4\n1\n0\n");
    std::istringstream contactInput(contactAnswers);std::ostringstream contactOutput;
    auto contact=BRExperimentMenu().choose(contactInput,contactOutput,defaults);
    require(contact.confirmed && contact.options.decoder==PackingDecoder::Contact && contact.options.candidates==DBLFCandidatePolicy::ThreePoints,"Contact menu selection failed");
    auto maximalAnswers = answers();
    maximalAnswers.replace(contactPosition, 10, "3\n5\n4\n1\n0\n");
    std::istringstream maximalInput(maximalAnswers);
    std::ostringstream maximalOutput;
    auto maximal = BRExperimentMenu().choose(maximalInput, maximalOutput, defaults);
    require(maximal.confirmed && maximal.options.decoder == PackingDecoder::MaximalSpaces &&
            maximal.options.candidates == DBLFCandidatePolicy::ThreePoints,
            "Maximal spaces menu selection failed");
    auto timedAnswers=answers();
    const auto modePosition=timedAnswers.find("4\n1\n0\n2\n123");
    require(modePosition!=std::string::npos,"Timed menu test fixture is invalid");
    timedAnswers.replace(modePosition,11,"4\n2\n2\n123");
    std::istringstream timedInput(timedAnswers);std::ostringstream timedOutput;
    auto timed=BRExperimentMenu().choose(timedInput,timedOutput,defaults);
    require(timed.confirmed && timed.options.execution==ExecutionMode::ThirtySeconds && timed.options.rounds==2 && timed.options.seed==123,"Timed menu did not skip the iteration prompt correctly");
    require(output.str().find("12 ejecuciones")!=std::string::npos,"Menu preview has wrong workload size");
    std::istringstream invalid("abc\n99\n"+answers());std::ostringstream diagnostic;
    require(BRExperimentMenu().choose(invalid,diagnostic,defaults).confirmed && diagnostic.str().find("Valor invalido")!=std::string::npos,"Menu did not recover from invalid input");
    std::istringstream eof;std::ostringstream ignored;
    require(!BRExperimentMenu().choose(eof,ignored,defaults).confirmed,"EOF started a test");
    std::istringstream cancel("salir\n");require(!BRExperimentMenu().choose(cancel,ignored,defaults).confirmed,"Cancel started a test");
    std::string advanced="1\n2\n3\n4\n2\n3\n2\n1\n3\n4\n1\n2\n1\n456\nadvanced\n1\n2\n0.25\n0.2\n3\n1\n2\n1\n1\n8\n1\n1\n";
    std::istringstream advancedInput(advanced);auto a=BRExperimentMenu().choose(advancedInput,ignored,defaults);
    require(a.confirmed && a.options.dataset=="BR1" && a.options.firstInstance==3 && a.options.instances==2 && a.options.algorithm=="ABC","Range/ABC selection failed");
    require(a.options.mutationProbability==0.25 && a.options.rotationMutation==0.2 && a.options.sites==3 && a.options.eliteSites==1 && a.options.eliteBees==2 && a.options.nonEliteBees==1,"Advanced ABC parameters were lost");
    require(a.options.cacheEntries==8 && a.options.cacheBytes==1024*1024 && a.options.candidates==DBLFCandidatePolicy::BestOfBoth,"Advanced cache/candidate parameters were lost");
}
void reports(){
    const auto directory=createResultSessionDirectory("build/br-menu-tests","report");
    const auto second=createResultSessionDirectory("build/br-menu-tests","report");
    require(directory!=second,"Session directories collide");
    rejects([]{createResultSessionDirectory("build/br-menu-tests","../outside");});
    CommandLineOptions options;options.root=".";options.output=directory;options.dataset="BR0";options.algorithm="all";options.population=4;options.instances=1;
    ExperimentOutcome a;a.dataset="BR0";a.algorithm="GA";a.position=2;a.instanceId=2;a.round=1;a.seed=std::numeric_limits<std::uint64_t>::max();a.datasetSeed=2502605;a.fitness=0.5;a.milliseconds=10;a.checkpoint="a,\"quoted\".checkpoint";
    auto b=a;b.round=2;b.fitness=0.75;b.milliseconds=20;b.resumed=true;
    auto c=a;c.algorithm="ABC";c.fitness=0;c.milliseconds=0;
    b.termination="time_limit";b.partialEvaluation=true;
    std::ostringstream terminal;terminal<<std::scientific<<std::setprecision(1);
    const auto flags=terminal.flags();const auto precision=terminal.precision();
    printExperimentSummary(terminal,{a,b,c});
    const auto printed=terminal.str();
    require(printed.find("OCUPACION FINAL: 75.000 %")!=std::string::npos && printed.find("OCUPACION FINAL: 0.000 %")!=std::string::npos,"Final occupation percentage is not clearly displayed");
    require(printed.find("RESULTADOS FINALES")!=std::string::npos && printed.find("ocupacion: 75.000 %")!=std::string::npos && printed.find("tiempo: 20 ms")!=std::string::npos,"Terminal summary lost fitness/time");
    require(printed.find("limite de tiempo")!=std::string::npos && printed.find("evaluacion parcial")!=std::string::npos && printed.find("recuperado del archivo")!=std::string::npos && printed.find("Total: 3 resultados")!=std::string::npos,"Terminal summary lost termination/resume metadata");
    require(terminal.flags()==flags && terminal.precision()==precision,"Terminal summary changed stream formatting");
    BRExperimentSession().writeReport(directory,options,{a,b,c});
    const auto text=read(directory+"/resumen.md"),csv=read(directory+"/resultados.csv");
    require(text.find("### BR0 / GA")!=std::string::npos && text.find("### BR0 / ABC")!=std::string::npos,"Report has no algorithm sections");
    require(text.find("62.500")!=std::string::npos && text.find("15.000")!=std::string::npos && text.find("Se guardaron 3 resultados")!=std::string::npos,"Report aggregation is incorrect");
    require(csv.find("18446744073709551615")!=std::string::npos && csv.find("\"a,\"\"quoted\"\".checkpoint\"")!=std::string::npos,"CSV lost seed precision or quoting");
    require(csv.find("\"ABC\"")!=std::string::npos && text.find("0.000")!=std::string::npos,"Zero results were omitted");
    require(BRExperimentSession().execute(BRMenuSelection{}).empty(),"Cancelled menu wrote a session");
    BRMenuSelection invalid;invalid.confirmed=true;invalid.options=options;invalid.options.dataset="P1A2";
    rejects([&]{BRExperimentSession().execute(invalid);});
}
void execution(){
    BRMenuSelection selection;selection.confirmed=true;selection.sessionName="execution";
    auto &o=selection.options;o.root=".";o.output="build/br-menu-tests";o.dataset="BR18";o.firstInstance=2;o.instances=1;o.algorithm="GA";o.population=2;o.iterations=0;o.seed=123;
    const auto directory=BRExperimentSession().execute(selection);
    const auto csv=read(directory+"/resultados.csv");
    require(csv.find("\"BR18\",2,2,\"GA\"")!=std::string::npos,"Session ran the first problem instead of the selected one");
    const auto config=read(directory+"/configuracion.txt");
    require(config.find("--report")!=std::string::npos && config.find("--first-instance 2")!=std::string::npos,"Session has no resumable configuration");
    auto resume=o;resume.output=directory;BRExperimentSession().resume(resume);
    require(read(directory+"/resumen.md").find("| Resultados reanudados | 1 |")!=std::string::npos,"Resumed report did not preserve results");
    const auto root=createResultSessionDirectory("build/br-menu-tests","default-root");
    saveResultText(root+"/Instance/BR/BR0.txt","1\n1 123\n10 10 10\n1\n1 1 1 1 1 1 1 2\n");
    auto tiny=selection;tiny.options.root=root;tiny.options.output.clear();tiny.options.dataset="BR0";tiny.options.firstInstance=1;
    const auto defaultOutput=BRExperimentSession().execute(tiny);
    require(defaultOutput.find(root+"/Results/BR/")==0,"Session ignored dedicated BR results directory");
    auto failed=tiny.options;failed.output=defaultOutput;failed.dataset="BR18";
    rejects([&]{BRExperimentSession().resume(failed);});
    require(read(defaultOutput+"/resumen.md").find("Estado: interrumpida")!=std::string::npos,"Failed session did not preserve a failure report");
}
int main(){
    try{menuTests();reports();execution();std::cout<<"PASS: BR menu choices, input recovery, ranges, session isolation, reports and resume\n";return 0;}
    catch(const std::exception &error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}

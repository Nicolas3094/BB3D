#include "BRExperimentSession.hpp"
#include "readData/ExperimentResults.hpp"
#include <cmath>
#include <iomanip>
#include <locale>
#include <map>

namespace {
bool brDataset(const std::string &name) {
    if(name=="BR")return true;
    for(int i=0;i<=18;++i)if(name=="BR"+std::to_string(i))return true;
    return false;
}
std::string quote(const std::string &value) {
    std::string result="'";for(char ch:value){result+=ch;if(ch=='\'')result+='\'';}return result+"'";
}
std::string csv(const std::string &value) {
    std::string result="\"";for(char ch:value){result+=ch;if(ch=='"')result+='"';}return result+'"';
}
std::string mutation(MutationType value) {return value==MutationType::INVERSE_MUTATION?"inversion":(value==MutationType::C1?"C1":"C2");}
std::string resumeCommand(const CommandLineOptions &o) {
    std::ostringstream out;out.imbue(std::locale::classic());out<<std::setprecision(17)<<"& './main.exe' --report";
    auto text=[&](const char *flag,const std::string &value){out<<' '<<flag<<' '<<quote(value);};
    text("--root",o.root);text("--output",o.output);text("--algorithm",o.algorithm);text("--dataset",o.dataset);
    text("--rotation",o.rotation);text("--mutation",o.mutation);text("--replacement",o.replacement);
    out<<" --iterations "<<o.iterations<<" --rounds "<<o.rounds<<" --first-instance "<<o.firstInstance<<" --instances "<<o.instances<<" --seed "<<o.seed;
    if(o.population!=-1)out<<" --population "<<o.population;
    if(o.sites!=-1)out<<" --sites "<<o.sites;if(o.eliteSites!=-1)out<<" --elite-sites "<<o.eliteSites;
    out<<" --elite-bees "<<o.eliteBees<<" --non-elite-bees "<<o.nonEliteBees<<" --movements "<<o.movements;
    out<<" --selection "<<o.selection<<" --crossover "<<o.crossover<<" --diversity "<<o.diversity<<" --patience "<<o.patience;
    if(o.mutationProbability==-1)out<<" --mutation-probability adaptive";else out<<" --mutation-probability "<<o.mutationProbability;
    if(o.rotationMutation!=-1)out<<" --rotation-mutation "<<o.rotationMutation;
    text("--candidates",o.candidates==DBLFCandidatePolicy::ThreePoints?"legacy":(o.candidates==DBLFCandidatePolicy::Projected?"projected":"best"));
    out<<" --cache-entries "<<o.cacheEntries<<" --cache-mib "<<o.cacheBytes/(1024*1024);
    return out.str();
}
std::string configuration(const CommandLineOptions &o) {
    std::ostringstream out;out.imbue(std::locale::classic());
    out<<"Sesion de pruebas BR\nDataset: "<<o.dataset<<"\nAlgoritmo: "<<o.algorithm<<"\nPrimer problema: "<<o.firstInstance
       <<"\nCantidad: "<<(o.instances==0?"todos los restantes":std::to_string(o.instances))<<"\nMutacion: "<<o.mutation
       <<"\nOrientaciones: "<<o.rotation<<"\nReemplazo: "<<o.replacement<<"\nPoblacion: "<<o.population
       <<"\nCandidatos DBLF: "<<(o.candidates==DBLFCandidatePolicy::ThreePoints?"legacy":(o.candidates==DBLFCandidatePolicy::Projected?"projected":"best"))
       <<"\nIteraciones: "<<o.iterations<<"\nRondas: "<<o.rounds<<"\nSemilla raiz: "<<o.seed
       <<"\nResultados: "<<o.output<<"\n\nComando para reanudar desde la carpeta del proyecto (PowerShell):\n"<<resumeCommand(o)<<'\n';
    return out.str();
}
}
void BRExperimentSession::writeReport(const std::string &directory,const CommandLineOptions &o,const std::vector<ExperimentOutcome> &rows) const {
    if(rows.empty())throw std::invalid_argument("No hay resultados para esta sesion BR.");
    std::ostringstream details,report;details.imbue(std::locale::classic());report.imbue(std::locale::classic());
    details<<std::setprecision(17)<<"dataset,problema,id,algoritmo,rotaciones,mutacion,reemplazo,ronda,semilla_dataset,semilla_ejecucion,fitness,ocupacion_porcentaje,tiempo_ms,reanudado,checkpoint\n";
    report<<"# Sesion de pruebas BR\n\nEstado: completada\n\n## Configuracion\n\n"
          <<"- Dataset: "<<o.dataset<<"\n- Algoritmo: "<<o.algorithm<<"\n- Primer problema: "<<o.firstInstance
          <<"\n- Cantidad de problemas: "<<(o.instances==0?"todos los restantes":std::to_string(o.instances))
          <<"\n- Mutacion: "<<o.mutation<<"\n- Orientaciones: "<<o.rotation<<"\n- Reemplazo: "<<o.replacement
          <<"\n- Candidatos DBLF: "<<(o.candidates==DBLFCandidatePolicy::ThreePoints?"legacy":(o.candidates==DBLFCandidatePolicy::Projected?"projected":"best"))
          <<"\n- Poblacion: "<<o.population<<"\n- Iteraciones: "<<o.iterations<<"\n- Rondas: "<<o.rounds<<"\n- Semilla raiz: "<<o.seed
          <<"\n- Resultados individuales: [resultados.csv](resultados.csv)\n- Parametros y comando de reanudacion: [configuracion.txt](configuracion.txt)\n\n## Resultados por algoritmo y metodo\n\n";
    std::map<std::string,std::vector<const ExperimentOutcome *>> groups;
    for(const auto &row:rows) {
        if(!brDataset(row.dataset) || !std::isfinite(row.fitness) || row.fitness<0 || row.fitness>1 || row.milliseconds<0)
            throw std::invalid_argument("Resultado BR invalido.");
        const auto group=row.dataset+" / "+row.algorithm+" / rotaciones "+std::to_string(static_cast<int>(row.rotation))+" / "+mutation(row.mutation)+" / "+(row.replacement?"con reemplazo":"sin reemplazo");
        groups[group].push_back(&row);
        details<<csv(row.dataset)<<','<<row.position<<','<<row.instanceId<<','<<csv(row.algorithm)<<','<<static_cast<int>(row.rotation)<<','<<csv(mutation(row.mutation))<<','<<row.replacement<<','<<row.round<<','<<row.datasetSeed<<','<<row.seed<<','<<row.fitness<<','<<100*row.fitness<<','<<row.milliseconds<<','<<row.resumed<<','<<csv(row.checkpoint)<<'\n';
    }
    report<<std::fixed<<std::setprecision(3);
    for(const auto &group:groups) {
        long double totalFitness=0,totalTime=0;double best=0,worst=1;size_t resumed=0;
        for(const auto *row:group.second){totalFitness+=row->fitness;totalTime+=row->milliseconds;best=std::max(best,row->fitness);worst=std::min(worst,row->fitness);resumed+=row->resumed?1:0;}
        report<<"### "<<group.first<<"\n\n| Medida | Resultado |\n| --- | ---: |\n"
              <<"| Ejecuciones | "<<group.second.size()<<" |\n| Ocupacion media (%) | "<<100*totalFitness/group.second.size()
              <<" |\n| Mejor ocupacion (%) | "<<100*best<<" |\n| Peor ocupacion (%) | "<<100*worst
              <<" |\n| Tiempo medio (ms) | "<<totalTime/group.second.size()<<" |\n| Tiempo total (ms) | "<<totalTime
              <<" |\n| Resultados reanudados | "<<resumed<<" |\n\n";
    }
    report<<"## Detalle\n\nSe "<<(rows.size()==1?"guardo ":"guardaron ")<<rows.size()<<(rows.size()==1?" resultado.":" resultados.")<<" El CSV identifica cada problema, ronda, semilla, metodo y checkpoint.\n"
          <<"Los tiempos de resultados reanudados son los de su ejecucion original.\n";
    saveResultText(directory+"/resultados.csv",details.str());
    saveResultText(directory+"/resumen.md",report.str());
}
std::string BRExperimentSession::execute(const BRMenuSelection &selection) const {
    if(!selection.confirmed)return {};
    auto o=selection.options;
    if(!brDataset(o.dataset) || o.root.empty())throw std::invalid_argument("El menu de sesiones solo admite datasets BR con una raiz configurada.");
    const auto algorithms=o.algorithm=="all"?std::vector<std::string>{"GA","ABC","FFA"}:std::vector<std::string>{o.algorithm};
    for(const auto &name:algorithms)o.configuration(name,ROTATION_WAY::ZERO_WAY,MutationType::INVERSE_MUTATION,true);
    const auto base=o.output.empty()?o.root+"/Results/BR":o.output;
    o.output=createResultSessionDirectory(base,selection.sessionName);
    return resume(o);
}
std::string BRExperimentSession::resume(const CommandLineOptions &o) const {
    if(!brDataset(o.dataset) || o.root.empty() || o.output.empty())throw std::invalid_argument("El informe BR requiere un dataset BR, raiz y carpeta de salida explicita.");
    const auto directory=o.output;
    try {
        saveResultText(directory+"/configuracion.txt",configuration(o));
        saveResultText(directory+"/resumen.md","# Sesion de pruebas BR\n\nEstado: en curso. Los checkpoints conservan el avance.\n");
        const auto rows=runExperiments(o);writeReport(directory,o,rows);
    } catch(const std::exception &error) {
        try {saveResultText(directory+"/resumen.md",std::string("# Sesion de pruebas BR\n\nEstado: interrumpida\n\n")+error.what()+"\n\nConsulta configuracion.txt para reanudar desde los checkpoints.\n");}catch(...){}
        throw std::runtime_error(std::string(error.what())+"; sesion: "+directory);
    }
    return directory;
}

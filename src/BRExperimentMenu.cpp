#include "BRExperimentMenu.hpp"
#include "readData/ReadData.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <locale>

namespace {
struct Cancelled {};
std::string line(std::istream &input) {
    std::string value;if(!std::getline(input,value))throw Cancelled{};
    const auto first=value.find_first_not_of(" \t\r\n");
    value=first==std::string::npos?"":value.substr(first,value.find_last_not_of(" \t\r\n")-first+1);
    if(value=="salir" || value=="cancelar")throw Cancelled{};
    return value;
}
int number(std::istream &input,std::ostream &output,const std::string &label,int minimum,int maximum,int fallback) {
    fallback=std::max(minimum,std::min(maximum,fallback));
    for(;;) {
        output<<label<<" ["<<fallback<<"]: "<<std::flush;const auto text=line(input);if(text.empty())return fallback;
        std::istringstream parse(text);long long value;std::string extra;
        if(parse>>value && !(parse>>extra) && value>=minimum && value<=maximum)return static_cast<int>(value);
        output<<"Valor invalido. Usa un entero entre "<<minimum<<" y "<<maximum<<".\n";
    }
}
double probability(std::istream &input,std::ostream &output,const std::string &label,double fallback) {
    for(;;) {
        output<<label<<" ["<<fallback<<"]: "<<std::flush;const auto text=line(input);if(text.empty())return fallback;
        std::istringstream parse(text);parse.imbue(std::locale::classic());double value;std::string extra;
        if(parse>>value && !(parse>>extra) && std::isfinite(value) && value>=0 && value<=1)return value;
        output<<"Valor invalido. Usa una probabilidad entre 0 y 1 (punto decimal).\n";
    }
}
int problemCount(const std::string &root,int file) {
    const auto path=root+"/Instance/BR/BR"+std::to_string(file)+".txt";
    std::ifstream input(path);if(!input)throw std::runtime_error("No se puede abrir "+path);
    std::string text;bool first=true;
    while(std::getline(input,text)) {
        if(first && text.compare(0,3,"\xEF\xBB\xBF")==0)text.erase(0,3);first=false;
        const auto values=getNumbersFromLines(text);if(values.empty())continue;
        if(values.size()!=1 || values[0]<=0)throw std::invalid_argument("Cabecera BR invalida: "+path);
        return values[0];
    }
    throw std::invalid_argument("Archivo BR vacio: "+path);
}
}
BRMenuSelection BRExperimentMenu::choose(std::istream &input,std::ostream &output,const CommandLineOptions &defaults) const {
    BRMenuSelection result;result.options=defaults;auto &o=result.options;o.help=false;o.interactive=true;
    try {
        output<<"\n=== Pruebas BR ===\nEscribe salir para cancelar; Enter conserva el valor sugerido.\n"
              <<"Archivos: 0=BR0, 1=BR1, ... 18=BR18, 19=todos los BR.\n";
        const int file=number(input,output,"Archivo",0,19,0);o.dataset=file==19?"BR":"BR"+std::to_string(file);
        const int count=problemCount(o.root,file==19?0:file);
        output<<"Problemas: 1=uno, 2=rango, 3=todos ("<<count<<").\n";
        const int selection=number(input,output,"Seleccion de problemas",1,3,1);
        o.firstInstance=selection==3?1:number(input,output,"Primer problema (posicion en el archivo)",1,count,1);
        o.instances=selection==3?0:1;
        if(selection==2)o.instances=number(input,output,"Ultimo problema",o.firstInstance,count,o.firstInstance+std::min(9,count-o.firstInstance))-o.firstInstance+1;
        output<<"Algoritmos: 1=Genetico (GA), 2=Abejas (ABC), 3=Firefly (FFA), 4=comparar los tres, 5=articulo SCLP (Guesser 2026).\n";
        const int algorithm=number(input,output,"Algoritmo",1,5,1);o.algorithm=std::vector<std::string>{"GA","ABC","FFA","all","SCLP"}[algorithm-1];
        if(o.algorithm=="SCLP") {
            o.execution=ExecutionMode::ThirtySeconds;
            output<<"SCLP: configuracion publicada de dos etapas, memoizacion strict, 1 hilo.\n"
                  <<"30 segundos CPU por problema y ronda, incluyendo generacion de bloques.\n"
                  <<"Orientaciones: todos los permisos de los datos BR. No utiliza mutacion, poblacion ni DBLF.\n";
            o.rounds=number(input,output,"Rondas",1,10000,o.rounds);
            for(;;) {
                output<<"Nombre de sesion (letras, numeros, guion) [sesion]: "<<std::flush;
                auto name=line(input);if(name.empty())name="sesion";
                if(name.size()<=64 && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-")==std::string::npos){result.sessionName=name;break;}
                output<<"Nombre invalido.\n";
            }
            const auto runs=static_cast<std::uint64_t>(file==19?19:1)*(o.instances==0?count:o.instances)*o.rounds;
            output<<"\nResumen: "<<o.dataset<<", desde problema "<<o.firstInstance<<", algoritmo SCLP, "<<runs<<" ejecuciones de hasta 30 segundos CPU.\n";
            result.confirmed=number(input,output,"Iniciar? 1=si, 0=cancelar",0,1,0)==1;
            if(!result.confirmed)output<<"Sesion cancelada; no se escribieron resultados.\n";
            return result;
        }
        output<<"Mutacion: 1=inversion, 2=C1, 3=C2, 4=comparar todas.\n";
        o.mutation=std::vector<std::string>{"inverse","c1","c2","all"}[number(input,output,"Metodo de mutacion",1,4,1)-1];
        output<<"Orientaciones: 1=sin rotacion, 2=dos, 3=seis, 4=comparar todas.\n";
        o.rotation=std::vector<std::string>{"0","2","6","all"}[number(input,output,"Orientaciones",1,4,3)-1];
        output<<"Reemplazo: 1=con reemplazo, 2=sin reemplazo, 3=ambos.\n";
        o.replacement=std::vector<std::string>{"true","false","both"}[number(input,output,"Reemplazo",1,3,1)-1];
        output<<"Decodificador: 1=DBLF legacy, 2=DBLF proyectado, 3=DBLF best, 4=contacto (tres puntos originales), 5=Espacios Maximos (DBL).\n";
        int defaultDecoderOption = 1;
        if (o.decoder == PackingDecoder::Contact)
            defaultDecoderOption = 4;
        if (o.decoder == PackingDecoder::MaximalSpaces)
            defaultDecoderOption = 5;
        const int decoderOption = number(input, output, "Decodificador de empacado", 1, 5, defaultDecoderOption);
        o.candidates = DBLFCandidatePolicy::ThreePoints;
        switch (decoderOption)
        {
        case 4:
            o.decoder = PackingDecoder::Contact;
            break;
        case 5:
            o.decoder = PackingDecoder::MaximalSpaces;
            break;
        default:
            o.decoder = PackingDecoder::DBLF;
            o.candidates = static_cast<DBLFCandidatePolicy>(decoderOption - 1);
            break;
        }
        o.population=number(input,output,"Tamano de poblacion",2,100000,o.population>=2?o.population:20);
        output<<"Ejecucion: 1=normal por iteraciones, 2=hasta 30 segundos por problema/algoritmo/ronda.\n";
        o.execution=number(input,output,"Modo de ejecucion",1,2,o.execution==ExecutionMode::ThirtySeconds?2:1)==2?ExecutionMode::ThirtySeconds:ExecutionMode::Iterations;
        if(o.execution==ExecutionMode::Iterations)o.iterations=number(input,output,"Maximo de iteraciones",0,10000000,o.iterations);
        o.rounds=number(input,output,"Rondas independientes",1,10000,o.rounds);
        for(;;) {
            output<<"Semilla raiz ["<<o.seed<<"]: "<<std::flush;const auto text=line(input);if(text.empty())break;
            try{o.seed=parseExperimentSeed(text);break;}catch(const std::exception &){output<<"Semilla invalida: usa un entero sin signo.\n";}
        }
        for(;;) {
            output<<"Nombre de sesion (letras, numeros, guion) [sesion]: "<<std::flush;auto name=line(input);if(name.empty())name="sesion";
            if(name.size()<=64 && name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-")==std::string::npos) {result.sessionName=name;break;}
            output<<"Nombre invalido: maximo 64 caracteres, sin rutas ni espacios.\n";
        }
        if(number(input,output,"Configurar parametros avanzados? 1=si, 0=no",0,1,0)) {
            if(o.algorithm=="GA" || o.algorithm=="all") {
                o.selection=probability(input,output,"Probabilidad de seleccionar al mejor",o.selection);
                o.crossover=probability(input,output,"Probabilidad de cruce",o.crossover);
                if(o.execution==ExecutionMode::Iterations) {
                    o.diversity=probability(input,output,"Umbral de diversidad genetica",o.diversity);
                    o.patience=number(input,output,"Generaciones sin mejora (0 desactiva parada)",0,10000000,o.patience);
                }
            }
            const int mode=number(input,output,"Mutacion: 1=adaptativa en GA/1 en otros, 2=probabilidad fija",1,2,1);
            o.mutationProbability=mode==1?-1:probability(input,output,"Probabilidad fija de mutacion",1);
            o.rotationMutation=probability(input,output,"Probabilidad de mutacion de orientacion",o.algorithm=="GA"?0.05:0.01);
            if(o.algorithm=="ABC" || o.algorithm=="all") {
                o.sites=number(input,output,"Sitios ABC",0,o.population,std::min(11,o.population));
                o.eliteSites=number(input,output,"Sitios elite ABC",0,o.sites,std::min(8,o.sites));
                o.eliteBees=number(input,output,"Abejas por sitio elite",0,10000,o.eliteBees);
                o.nonEliteBees=number(input,output,"Abejas por sitio no elite",0,10000,o.nonEliteBees);
            }
            if(o.algorithm=="FFA" || o.algorithm=="all")o.movements=number(input,output,"Movimientos por luciernaga",0,10000,o.movements);
            if(number(input,output,"Activar cache de fitness? 1=si, 0=no",0,1,0)) {
                o.cacheEntries=number(input,output,"Maximo de entradas de cache",1,1000000,256);
                o.cacheBytes=static_cast<size_t>(number(input,output,"Presupuesto de payload de cache (MiB)",1,1024,8))*1024*1024;
            } else o.cacheEntries=0;
        }
        const auto algorithms=o.algorithm=="all"?std::vector<std::string>{"GA","ABC","FFA"}:std::vector<std::string>{o.algorithm};
        for(const auto &name:algorithms)o.configuration(name,ROTATION_WAY::ZERO_WAY,MutationType::INVERSE_MUTATION,true);
        const auto runs=static_cast<std::uint64_t>(file==19?19:1)*(o.instances==0?count:o.instances)*algorithms.size()*
            (o.mutation=="all"?3:1)*(o.rotation=="all"?3:1)*(o.replacement=="both"?2:1)*o.rounds;
        output<<"\nResumen: "<<o.dataset<<", problemas desde "<<o.firstInstance<<", cantidad "<<(o.instances==0?count:o.instances)
              <<", algoritmo "<<o.algorithm<<", mutacion "<<o.mutation<<", orientaciones "<<o.rotation
              <<", reemplazo "<<o.replacement<<", candidatos "<<(o.candidates==DBLFCandidatePolicy::ThreePoints?"legacy":(o.candidates==DBLFCandidatePolicy::Projected?"projected":"best"))
              <<", poblacion "<<o.population<<", ejecucion "<<(o.execution==ExecutionMode::ThirtySeconds?"30 segundos":"iteraciones")<<", rondas "<<o.rounds
              <<". Total: "<<runs<<" ejecuciones.\n";
        if(o.execution==ExecutionMode::ThirtySeconds)output<<"Limite de busqueda: 30 segundos por ejecucion; los metodos se ejecutan en serie. Puede finalizar antes con ocupacion 100%.\n";
        else output<<"Maximo de iteraciones: "<<o.iterations<<".\n";
        output<<"Decodificador seleccionado: "<<packingDecoderName(o.decoder)<<".\n";
        result.confirmed=number(input,output,"Iniciar? 1=si, 0=cancelar",0,1,0)==1;
        if(!result.confirmed)output<<"Sesion cancelada; no se escribieron resultados.\n";
    } catch(const Cancelled &) {output<<"\nMenu cancelado; no se escribieron resultados.\n";}
    return result;
}

// Independent driver: original search and timer, additional output metadata only.
#include "sclp/Core.hpp"
#include <chrono>
#include <iostream>
int main(int argc,const char *const argv[]) {
    using namespace sclp;
    try {
        const auto start=std::chrono::steady_clock::now();
        auto problem=std::make_shared<Problem>(argc,argv);
        if(problem->cli_parameters.io.help){problem->cli_parameters.help_message();return 0;}
        problem->initialize();
        problem->solve();
        if(!AllConstraints::check(*problem->input,problem->best_state->packing))
            throw std::runtime_error("Original constraint checks rejected the packing");
        Logs logs(problem);logs.run();
        auto result=FileLoader::json(problem->cli_parameters.io.output_file);
        std::size_t i=0;
        for(const auto &item:problem->best_state->packing.allocated_small_items()) {
            result["small_items"][i]["type_index"]=item.type().index();
            result["small_items"][i]["orientation"]=to_string(item.orientation());
            ++i;
        }
        auto &audit=result["appendix"]["bb3d_execution"];
        audit["cpu_seconds_including_blocks"]=problem->timer.elapsed();
        audit["driver_wall_seconds_before_final_export"]=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        audit["solver_threads"]=1;
        audit["compiler"]=__VERSION__;
        audit["clock"]="boost::timer process user + system CPU";
        std::ofstream out(problem->cli_parameters.io.output_file);
        out<<result.dump(2)<<'\n';out.close();
        if(!out)throw std::runtime_error("Failed to save annotated packing");
        std::cout<<"OCUPACION FINAL: "<<result["appendix"]["packing_statistics"]["volume_fraction_relative_to_large_object_volume"]<<" %\n";
        std::cout<<"Tiempo CPU (incluye bloques): "<<problem->timer.elapsed()<<" s\n";
        return 0;
    } catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
}

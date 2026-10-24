#include "../src/readData/ReadData.hpp"
#include "../src/readData/BRInstanceReader.hpp"
#include "../src/heuristics/DBLF.hpp"
#include <sstream>
#include <stdexcept>
#include <limits>
void require(bool condition,const char *message) { if(!condition) throw std::runtime_error(message); }
template<class F> void rejects(F operation) {
    bool rejected=false;try {operation();}catch(const std::exception &){rejected=true;}
    require(rejected,"Malformed BR input was accepted");
}
std::vector<DatasetBinBacking> parse(const std::string &text) {
    std::istringstream stream(text);return BRInstanceReader().read(stream,"fixture.br");
}
void fixtureTests() {
    auto data=parse("\xEF\xBB\xBF 2\r\n\n42 -7\r\n10 20 30\r\n2\r\n7 2 1 3 0 4 1 2\r\n9 5 0 6 1 7 0 1\r\n\n17 123\r\n8 9 10\r\n1\r\n5 1 1 2 1 3 1 1\r\n\t\n");
    require(data.size()==2 && data[0].id==42 && data[0].PSeed==-7 && data[1].id==17,"IDs/seeds or final instance were lost");
    require(data[0].itemsByTpe.size()==2 && data[0].totalItems.size()==3 && data[1].totalItems.size()==1,"Variable per-instance quantities were not expanded");
    require(data[0].itemsByTpe[0].getId()==7 && data[0].itemsByTpe[1].getTipo()==9,"Original type IDs changed");
    require(data[0].totalItems[0].getId()==1 && data[0].totalItems[1].getId()==2 && data[0].totalItems[2].getId()==3 && data[1].totalItems[0].getId()==1,"Individual IDs are not dense per instance");
    require(data[0].totalItems[1].getTipo()==7 && data[0].totalItems[2].getTipo()==9,"Quantity expansion lost box types");
    require(data[0].totalItems[0].getCurrentDimension()==Punto::Build(2,3,4) && data[0].totalItems[0].getRotations()==Punto::Build(1,0,1),"Columns were read in the P1-P5 order");
    require(data[0].bin.getDimensions().getVolumen()==6000 && data[0].bin.getLoadedItems().empty(),"Reader did not produce a fresh container");
    auto bin=data[0].bin;DBLF(bin,{1,2,3},{0,0,0},data[0].totalItems);
    require(bin.getNumberOfLoadedItems()==2 && bin.getLoadedVolume()==48,"Parsed quantities/orientations do not work with DBLF");
    for(const std::string &invalid:std::vector<std::string>{
        "", "0\n", "-1\n", "1 1\n", "1\n", "2147483647\n",
        "1\n0 1\n10 10 10\n1\n1 1 1 1 1 1 1 1\n",
        "1\n1 1\n0 10 10\n1\n1 1 1 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n0\n",
        "1\n1 1\n10 10 10\n2147483647\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 0\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 -1\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 2147483647\n",
        "1\n1 1\n10 10 10\n1\n1 0 1 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 1 2 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 -1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 1x\n",
        "1\n1 1\n10 10 10\n1\n1 2147483648 1 1 1 1 1 1\n",
        "1\n1 1\n2147483647 2147483647 2147483647\n1\n1 1 1 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 2147483647 1 2147483647 1 2147483647 1 1\n",
        "1\n1 1\n10 10 10\n2\n1 1 1 1 1 1 1 1\n1 1 1 1 1 1 1 1\n",
        "2\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 1\n1 2\n10 10 10\n1\n1 1 1 1 1 1 1 1\n",
        "1\n1 1\n10 10 10\n1\n1 1 1 1 1 1 1 1\n99\n"})
        rejects([&]{parse(invalid);});
    rejects([]{BRInstanceReader(0);});
    rejects([]{BRInstanceReader(static_cast<size_t>(std::numeric_limits<int>::max())+1);});
    rejects([]{std::istringstream input("1\n1 1\n10 10 10\n2\n1 1 1 1 1 1 1 2\n2 1 1 1 1 1 1 2\n");BRInstanceReader(3).read(input);});
    rejects([]{BRInstanceReader().read("Instance/BR/nonexistent-br-file.txt");});
    rejects([]{std::istringstream input;input.setstate(std::ios::badbit);BRInstanceReader().read(input);});
    try {parse("1\n1 1\n10 10 10\n1\n1 1 2 1 1 1 1 1\n");}
    catch(const std::invalid_argument &error) {require(std::string(error.what()).find("fixture.br:5:")!=std::string::npos,"Error lacks source and physical line number");}
}
void originalFiles() {
    BRInstanceReader reader;
    for(int file=0;file<=18;++file) {
        const std::string path="Instance/BR/BR"+std::to_string(file)+".txt";
        const auto data=reader.read(path);
        std::ifstream reference(path);int count;reference>>count;
        require(reference.good() && count==100 && data.size()==static_cast<size_t>(count),"Real BR file lost instances");
        for(int instance=0;instance<count;++instance) {
            int id,seed,l,w,h,types;reference>>id>>seed>>l>>w>>h>>types;
            const auto &actual=data[instance];
            require(actual.id==id && actual.PSeed==seed && actual.bin.getDimensions().getLargo()==l && actual.bin.getDimensions().getAncho()==w && actual.bin.getDimensions().getAlto()==h,"Real BR instance header mismatch");
            require(actual.itemsByTpe.size()==static_cast<size_t>(types),"Real BR type count mismatch");
            size_t total=0;
            for(int type=0;type<types;++type) {
                int row[8];for(auto &value:row)reference>>value;
                require(!reference.fail(),"Reference BR fixture is truncated");
                const auto &box=actual.itemsByTpe[type];
                require(box.getTipo()==row[0] && box.getCurrentDimension()==Punto::Build(row[1],row[3],row[5]) && box.getRotations()==Punto::Build(row[2],row[4],row[6]),"Real BR item mapping mismatch");
                for(int copy=0;copy<row[7];++copy) {
                    require(total<actual.totalItems.size(),"BR expansion lost boxes");
                    const auto &item=actual.totalItems[total];
                    require(item.getId()==static_cast<int>(total+1) && item.getTipo()==row[0] && item.getCurrentDimension()==box.getCurrentDimension() && item.getRotations()==box.getRotations(),"Real BR expansion mismatch");
                    ++total;
                }
            }
            require(total==actual.totalItems.size(),"BR expansion added boxes");
        }
        int extra;require(!(reference>>extra),"Reference BR fixture has extra records");
    }
}
int main() {
    try {
        const auto data=readData("Instance/BR/BR0.txt");
        require(data.size()==100,"BR0 lost instances");
        require(data[0].totalItems.size()==122,"BR quantity must be read from the last column");
        const auto &item=data[0].totalItems[0];
        require(item.getCurrentDimension()==Punto::Build(108,76,30) && item.getRotations()==Punto::Build(0,0,1),"BR dimensions and orientation flags were transposed");
        fixtureTests();originalFiles();
        require(readData("Instance/P1A2.csv").size()==100,"BR dispatch broke P1-P5 datasets");
        std::cout<<"PASS: BR0-BR18 (1900 instances), exact expansion, rotations, malformed inputs and P1 compatibility\n";return 0;
    } catch(const std::exception &error) {std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}

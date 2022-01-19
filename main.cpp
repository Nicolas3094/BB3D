#include "main.hpp"

void LEER_DATA()
{
    cadena const PATH_IN = "data\\Boxes\\series1\\1ULD\\27boxes\\box4.csv";

    std::ifstream fl(PATH_IN);

    int DIM = 3;
    cadena str;
    int i = 0;
    std::map<cadena, bool> VisitTipo;

    while (std::getline(fl, str, '\n'))
    {
        int d = 0;
        double vol = 1;
        cadena num;
        Caja caja;
        Dim dimen;
        caja.id = i++;
        for (auto ch : str)
        {
            if (ch == ';')
            {
                d++;
                if (d == 1)
                {
                    dimen.l = stof(num);
                }
                else if (d == 2)
                {
                    dimen.w = stof(num);
                }
                else
                {
                    dimen.h = stof(num);
                }
                num.clear();
                continue;
            }
            if (d == DIM)
                break;
            num += ch;
        }
        dimen.vol = dimen.l * dimen.h * dimen.w;
        std::ostringstream strs;
        strs << dimen.l << dimen.h << dimen.w;
        if (!VisitTipo[strs.str()])
        {
            VisitTipo[strs.str()] = true;
            Bin::tipos.push_back(dimen);
            caja.tipo = Bin::tipos.size() - 1;
        }
        else
        {
            for (int i = 0; i < Bin::tipos.size(); i++)
            {
                if (Bin::tipos[i] == dimen)
                {
                    caja.tipo = i;
                    break;
                }
            }
        }
        Bin::cajas.push_back(caja);
        str.clear();
    }
    fl.close();
}

bool esEmpaquetable(Dim tipo, Punto pos, std::vector<Caja> loaded)
{
    bool boundCont = tipo.h + pos.z > Bin::ALTO || tipo.w + pos.y > Bin::ANCHO || tipo.l + pos.x > Bin::PROFUNDIDAD;
    if (boundCont)
        return false;

    return !Intersec(tipo, pos, loaded);
}

bool Intersec(Dim caja, Punto punto, std::vector<Caja> loaded)
{
    double px, py, pz, x, y, z;

    for (auto load : loaded)
    {
        py = load.pos.y + Bin::tipos[load.tipo].w;
        pz = load.pos.z + Bin::tipos[load.tipo].h;
        px = load.pos.x + Bin::tipos[load.tipo].l;
        y = punto.y + caja.w;
        x = punto.x + caja.l;
        z = punto.z + caja.h;

        if ((punto.y <= load.pos.y || punto.y < py) &&
            (punto.z <= load.pos.z || punto.z < pz) &&
            (punto.x <= load.pos.x || punto.x < px))
        {
            return true;
        }
    }

    return false;
}

cadena ToStrCode(Code cod)
{
    cadena nw = "";
    for (int el : cod)
    {
        nw += std::to_string(el);
    }
    return nw;
}

cadena strPunto(Punto pto)
{
    std::ostringstream strs;
    strs << pto.x << pto.y << pto.z;

    return strs.str();
}

void TDLF(Individuo &ind)
{
    Punto actual;
    Code unpacked = ind.code;
    ColaPuntos P;
    P.push(Punto(), DIRECCION::Z);
    int i; bool packing;

    while (!unpacked.empty() && !P.empty())
    {
        packing=false;
        actual = P.top();
        P.pop();

        for ( i = 0; i < unpacked.size(); i++)
        {
            if (esEmpaquetable(Bin::tipos[unpacked[i]], actual, ind.cont.cajas))
            {
                packing=true;
                break;
            }
        }
        if(packing){
            ind.cont.cajas.push_back(Caja(unpacked[i], actual));
            ind.cont.carga++;
            ind.cont.volCarga += Bin::tipos[unpacked[i]].vol;
            P.push(Punto(actual.x, actual.y, actual.z + Bin::tipos[unpacked[i]].h), DIRECCION::Z);
            P.push(Punto(actual.x, actual.y + Bin::tipos[unpacked[i]].w, actual.z), DIRECCION::Y);
            P.push(Punto(actual.x + Bin::tipos[unpacked[i]].l, actual.y, actual.z), DIRECCION::X);
            unpacked.erase(unpacked.begin() + i);
        }else{
            if(actual.z==0){
                P.top().z=0;
            }
        }
        
    }
    ind.fi = ind.cont.volCarga / (Bin::ALTO * Bin::ANCHO * Bin::PROFUNDIDAD);
    if (ind.fi < 0)
        ind.fi = 0;
}

void PobInicialH(std::vector<Individuo> &Pob, long N)
{
    Individuo original;
    std::random_device rd;
    std::mt19937 g(rd());
    original.cont.carga = 0;
    original.fi = 0;
    original.cont.volCarga = 0;
    original.code = cod(Bin::cajas);
    Pob.push_back(original);
    std::map<cadena, bool> visit;
    visit[ToStrCode(original.code)] = true;
    for (int i = 0; i < 4; i++)
    {
        std::sort(original.code.begin(), original.code.end(), [i](int ptr1, int ptr2)
                  { 
            switch (i){
                case 0: return Bin::tipos[ptr1].vol > Bin::tipos[ptr2].vol; 
                case 1: return Bin::tipos[ptr1].h > Bin::tipos[ptr2].h; 
                case 2: return Bin::tipos[ptr1].w > Bin::tipos[ptr2].w; 
                default: return Bin::tipos[ptr1].l > Bin::tipos[ptr2].l; 
            } });
        if (visit[ToStrCode(original.code)])
            continue;
        visit[ToStrCode(original.code)] = true;
        Pob.push_back(original);
    }
    for (int i = 0; i < N - 5; i++)
    {
        shuffle(original.code.begin(), original.code.end(), g);
        if (visit[ToStrCode(original.code)])
        {
            i--;
            continue;
        }
        Pob.push_back(original);
        visit[ToStrCode(original.code)] = true;
    }
}

void exportJSON(Individuo ind)
{
    std::ofstream outfile("C:\\Users\\nicoo\\Unity\\New Unity Project\\Assets\\Scenes\\BPP.json");
    outfile << "{" << std::endl;
    outfile << "\t \"box\" : ["
            << "\n";
    int i = 0;
    for (auto in : ind.cont.cajas)
    {
        outfile << "\t\t {"
                << "\n";

        outfile << "\t\t\t \"dimension\" :{";
        outfile << "\"x\":" << Bin::tipos[in.tipo].l << ",";
        outfile << "\"y\":" << Bin::tipos[in.tipo].h << ",";
        outfile << "\"z\":" << Bin::tipos[in.tipo].w;
        outfile << "},"
                << "\n";

        outfile << "\t\t\t \"position\" :{";
        outfile << "\"x\":" << in.pos.x << ",";
        outfile << "\"y\":" << in.pos.z << ",";
        outfile << "\"z\":" << in.pos.y;
        outfile << "}"
                << "\n";

        if (i < ind.cont.cajas.size() - 1)
            outfile << "\t\t },"
                    << "\n";
        else
            outfile << "\t\t }"
                    << "\n";
        i++;
    }
    outfile << "\t]"
            << "\n";
    outfile << "}";
    outfile.close();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    Bin::ALTO = 640; Bin::PROFUNDIDAD = 1250;
    Bin::ANCHO = 604;
    LEER_DATA();

    std::vector<Individuo> pob;

    PobInicialH(pob, 50);

    for(auto& individuo : pob){
        TDLF(individuo);
        print(individuo.fi);
    }
    exportJSON(pob[10]);
    //print(pob[10].fi);
   // print(promedioFI);

    return 0;
}
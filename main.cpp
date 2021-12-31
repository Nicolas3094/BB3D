#include "main.hpp"

bool operator==(Dim &d1, Dim &d2)
{
    return (d1.h == d2.h && d1.l == d2.l && d1.w == d2.w);
}
bool operator!=(Dim &d1, Dim &d2)
{
    return !(d1.h == d2.h && d1.l == d2.l && d1.w == d2.w);
}
std::ostream &operator<<(std::ostream &COUT, Dim &DAT)
{
    COUT << "(" << DAT.l << "," << DAT.w << "," << DAT.h << ")";
    return COUT;
}
std::ostream &operator<<(std::ostream &COUT, Code &DAT)
{
    for (unsigned i = 0; i < DAT.size(); i++)
    {
        COUT << DAT[i];
        if (i != DAT.size() - 1)
        {
            COUT << " ";
        }
    }
    return COUT;
}

Universo LEER_DATA()
{
    cadena const PATH_IN = "data\\Boxes\\series1\\1ULD\\27boxes\\box4.csv";
    std::ifstream fl(PATH_IN);
    Universo univ;

    int DIM = 3;
    cadena str;
    int i = 0;
    std::map<cadena, bool> VisitTipo;
    std::vector<Dim> tipoc;
    std::vector<Caja> cajas;
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
            tipoc.push_back(dimen);
            caja.tipo = tipoc.size() - 1;
        }
        else
        {
            for (int i = 0; i < tipoc.size(); i++)
            {
                if (tipoc[i] == dimen)
                {
                    caja.tipo = i;
                    break;
                }
            }
        }
        cajas.push_back(caja);
        str.clear();
    }
    fl.close();
    univ.cajas = cajas;
    univ.tipos = tipoc;
    for (int i = 0; i < univ.cajas.size(); i++)
    {
        double v = (univ.cajas[i].tipo);
    }
    return univ;
}

bool esEmpaquetable(int tipo, Coord pos, Universo unv)
{
    if (unv.tipos[tipo].h + pos.z > ALTO || unv.tipos[tipo].w + pos.y > ANCHO || unv.tipos[tipo].l + pos.x > LONGITUD)  return false;

    return true;
}
bool esCombinable(int tipo, Coord pos, Universo unv)
{
}
Code cod(Universo un)
{
    Code ct;
    for (int i = 0; i < un.cajas.size(); i++)
    {
        ct.push_back(un.cajas[i].tipo);
    }
    return ct;
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

void ActualizarP(Universo unv,Individuo& ind,std::vector<Coord> &P, int i, int j)
{
    Caja caja;
    caja.id = i;
    caja.pos = P[j];
    caja.tipo = ind.code[i];
    ind.cont.cajas.push_back(caja);
    ind.cont.carga++;
    ind.cont.volCarga += unv.tipos[ind.code[i]].vol;

    Coord nwcrd = P[j];
    nwcrd.z += unv.tipos[ind.code[i]].h;
    P.push_back(nwcrd);
    nwcrd.z -= unv.tipos[ind.code[i]].h;
    nwcrd.y += unv.tipos[ind.code[i]].w;
    P.push_back(nwcrd);
    nwcrd.y -= unv.tipos[ind.code[i]].w;
    nwcrd.x += unv.tipos[ind.code[i]].l;
    P.push_back(nwcrd);

    P.erase(P.begin() + j);
}

void ExtDBLF(Individuo& ind, Universo unv){
    Coord o;
    int k;
    bool boo1, boo2;

    o.x = 0;
    o.y = 0;
    o.z = 0;
    std::vector<Coord> P;
    P.push_back(o);

    for (int i = 0; i < ind.code.size(); i++){
        boo1 = false;
        boo2 = false;
        std::sort(P.begin(), P.end(), [](Coord ptr1, Coord ptr2){ return ptr1.z < ptr2.z; });
        for (int j = 0; j < P.size(); j++)
        {
            if (esEmpaquetable(ind.code[i], P[j], unv))
            {
                if (!boo1)
                {
                    k = j;
                    boo1 = true;
                }
                if (esCombinable(ind.code[i], P[j], unv))
                {
                    boo2 = true;
                    ActualizarP(unv,ind, P,i,j);
                    break;
                }
            }
        }
        if (!boo1)  continue;
        else if (!boo2)
        {
            ActualizarP(unv,ind, P,i,k);
        }
    }
    ind.fi = ind.cont.volCarga > (ALTO * ANCHO * LONGITUD) ? 0 : ind.cont.volCarga / (ALTO * ANCHO * LONGITUD);
}

void PobInicialH(Universo cnt, std::vector<Individuo> &Pob, long N)
{
    Individuo original;
    std::random_device rd;
    std::mt19937 g(rd());
    original.cont.carga = 0;
    original.fi = 0;
    original.cont.volCarga = 1;
    original.code = cod(cnt);
    Pob.push_back(original);
    std::map<cadena, bool> visit;
    visit[ToStrCode(original.code)] = true;
    for (int i = 0; i < 4; i++)
    {
        std::sort(original.code.begin(), original.code.end(), [cnt, i](int ptr1, int ptr2)
                  { 
            switch (i){
                case 0: return cnt.tipos[ptr1].vol > cnt.tipos[ptr2].vol; 
                case 1: return cnt.tipos[ptr1].h > cnt.tipos[ptr2].h; 
                case 2: return cnt.tipos[ptr1].w > cnt.tipos[ptr2].w; 
                default: return cnt.tipos[ptr1].l > cnt.tipos[ptr2].l; 
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

int main()
{
    srand(time(NULL));
    Universo un = LEER_DATA();
    std::vector<Individuo> pob;
    PobInicialH(un, pob, 100);
    print(un.cajas.size());
    print(un.tipos.size());
    // FFit(ap, un);

    return 0;
}
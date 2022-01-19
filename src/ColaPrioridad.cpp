#include "ColaPrioridad.hpp"

ColaPuntos::ColaPuntos(){
    x=0; y=0; z=0;
}
Punto& ColaPuntos::top(){
    return this->cola.front();
}
Punto ColaPuntos::back(){
    return this->cola.back();
}
void ColaPuntos::pop(){
    if(z!=0) z--;
    else if(y!=0) y--;
    else x--;
    cola.erase(cola.begin());
}
void ColaPuntos::push(Punto pt,DIRECCION dir){

    if(dir==DIRECCION::X){
         x++;
         cola.push_back(pt);
    }
    else if(dir==DIRECCION::Y){
        cola.insert(cola.begin()+z+y,pt);
        y++;
    }
    else{ 
        cola.insert(cola.begin(),pt);
        z++;
    }
   
}
void ColaPuntos::printQ(){
    for(auto pt : cola){
        std::cout << "("<<pt.x<<","<<pt.y<<","<<pt.z<<")->";
    }
    std::cout <<"NULL\n";
}
int ColaPuntos::size(){
    return cola.size();
}
bool ColaPuntos::empty(){
    return cola.empty();
}
Punto& ColaPuntos::operator[](int index)
{
    return cola[index];
}
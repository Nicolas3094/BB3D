#include "ColaPrioridad.hpp"

ColaPuntosDBL::ColaPuntosDBL()
{
}

ColaPuntosDBL &ColaPuntosDBL::Build()
{
    ColaPuntosDBL *cola = new ColaPuntosDBL();
    return *cola;
}

Punto &ColaPuntosDBL::top()
{
    return this->cola.front();
}

Punto ColaPuntosDBL::back()
{
    return this->cola.back();
}

void ColaPuntosDBL::pop()
{
    if (cola.size() > 0)
    {
        cola.erase(cola.begin());
    }
}

void ColaPuntosDBL::clear()
{
    cola.clear();
}

void ColaPuntosDBL::push(Punto &pt)
{
    if (this->cola.size() == 0)
    {
        this->cola.push_back(pt);
        return;
    }
    if (cola.at(cola.size() - 1) == pt)
    {
        return;
    }
    if (condition(pt, cola.at(cola.size() - 1)))
    {
        this->cola.push_back(pt);
        return;
    }
    for (int i = this->cola.size() - 1; i >= 0; i--)
    {
        // std::cout << "punto: " << pt << "\n";
        // std::cout << "cola[i]: " << cola[i] << "\n";
        //  If satisfies condition, it is added at front.
        if (condition(pt, cola[i]))
        {
            // std::cout << "Se inserta " << pt << " adelante de: " << cola[i] << "\n";
            cola.insert(cola.begin() + i + 1, pt);
            return;
        }
        // Do not add if the point is already in list.
        if (pt == cola[i])
        {
            return;
        }
    }
    // Otherwise, it is inserted at the begining.
    cola.insert(cola.begin(), pt);
}

void ColaPuntosDBL::remove(int index)
{
    this->cola.erase(cola.begin() + index);
}

void ColaPuntosDBL::update()
{
    for (int i = this->cola.size() - 1; i >= 0; i--)
    {
        Punto q = cola[i];
        Punto qprev = cola[i - 1];
        if ((qprev.x == q.x && qprev.z == q.z) || (qprev.x < q.x && qprev.y == q.y && qprev.z == q.z))
        {
            // std::cout << "Will erase: " << cola.at(i - 1) << "\n";
            cola.erase(cola.begin() + i - 1);
        }
    }
}

void ColaPuntosDBL::printQ()
{
    for (auto pt : cola)
    {
        std::cout << "(" << pt.x << "," << pt.y << "," << pt.z << ")->";
    }
    std::cout << "NULL\n";
}

int ColaPuntosDBL::size()
{
    return cola.size();
}

bool ColaPuntosDBL::empty()
{
    return cola.empty();
}

Punto &ColaPuntosDBL::operator[](int index)
{
    return cola[index];
}

bool ColaPuntosDBL::condition1(Punto pt1, Punto pt2)
{
    return pt1.x < pt2.x;
}

bool ColaPuntosDBL::condition2(Punto pt1, Punto pt2)
{
    return pt1.x == pt2.x && pt1.z < pt2.z;
}

bool ColaPuntosDBL::condition3(Punto pt1, Punto pt2)
{
    return pt1.x == pt2.x && pt1.z == pt2.z && pt1.y < pt2.y;
}

bool ColaPuntosDBL::condition(Punto pt1, Punto pt2)
{
    return condition1(pt1, pt2) || condition2(pt1, pt2) || condition3(pt1, pt2);
}

std::ostream &operator<<(std::ostream &COUT, ColaPuntosDBL &cola)
{
    COUT << "|";
    for (int i = 0; i < cola.size(); i++)
    {
        COUT << cola[i] << "|";
    }
    COUT << "\n";
    return COUT;
}

#include "Node.hpp"

#include "../spatial/Punto.hpp"

template <typename T>
Node<T>::Node()
{
    this->next = null;
    this->previous = null;
}
template <typename T>
Node<T>::Node(T data)
{
    this->data = data;
    this->next = null;
    this->previous = null;
}
template <typename T>
Node<T>::Node(T data, Node *next)
{
    this->data = data;
    this->next = next;
    this->previous = null;
}
template <typename T>
Node<T>::~Node()
{
}

template class Node<Punto>;
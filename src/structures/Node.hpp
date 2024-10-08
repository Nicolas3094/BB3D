#ifndef NODE_H
#define NODE_H

#define null nullptr
#define REAR nullptr
#define HEAD nullptr

template <typename T>
class Node
{
public:
    T data;
    Node *next;
    Node *previous;
    Node();
    Node(T);
    Node(T, Node *);
    ~Node();
};

#endif

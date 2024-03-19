#ifndef NODE_H
#define NODE_H

#define null nullptr

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

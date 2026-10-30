#ifndef PRIORITY_QUEUE_HEADER
#define PRIORITY_QUEUE_HEADER

#include "Node.hpp"
#include "../spatial/Dimension.hpp"

class PriorityPointQueue
{
  private:
    int size;

    Node<Punto> *rear;

    bool condition(Punto, Punto);
    bool condition1(Punto, Punto);
    bool condition2(Punto, Punto);
    bool condition3(Punto, Punto);

  public:
    PriorityPointQueue();
    ~PriorityPointQueue();
    PriorityPointQueue(const PriorityPointQueue &);
    PriorityPointQueue &operator=(const PriorityPointQueue &);
    PriorityPointQueue(PriorityPointQueue &&) noexcept;
    PriorityPointQueue &operator=(PriorityPointQueue &&) noexcept;
    void swap(PriorityPointQueue &) noexcept;

    static PriorityPointQueue Build();

    void enqueue(Punto);
    void pop();
    void clear();
    void update();
    Node<Punto> *getRear();
    int getSize();
    bool isEmpty();
    void removeNode(Node<Punto> *);
    // currentNode must belong to this queue; insertion updates its size.
    void addNodeBeforeAt(Node<Punto> *, Punto);
};

std::ostream &operator<<(std::ostream &, PriorityPointQueue &);

#endif

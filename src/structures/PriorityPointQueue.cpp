#include "PriorityPointQueue.hpp"
#include <utility>
#include <stdexcept>

PriorityPointQueue::PriorityPointQueue()
{
    size = 0;
    rear = null;
}

PriorityPointQueue::~PriorityPointQueue()
{
    clear();
}

PriorityPointQueue::PriorityPointQueue(const PriorityPointQueue &other) : size(0), rear(nullptr)
{
    Node<Punto> *tail = nullptr;
    try
    {
        for (auto *source = other.rear; source; source = source->next)
        {
            auto *node = new Node<Punto>(source->data);
            node->previous = tail;

            if (tail)
                tail->next = node;
            else
                rear = node;
            tail = node;
            ++size;
        }
    }
    catch (...)
    {
        clear();
        throw;
    }
}

PriorityPointQueue &PriorityPointQueue::operator=(const PriorityPointQueue &other)
{
    if (this != &other)
    {
        PriorityPointQueue copy(other);
        swap(copy);
    }
    return *this;
}

PriorityPointQueue::PriorityPointQueue(PriorityPointQueue &&other) noexcept
    : size(other.size), rear(other.rear)
{
    other.size = 0;
    other.rear = nullptr;
}

PriorityPointQueue &PriorityPointQueue::operator=(PriorityPointQueue &&other) noexcept
{
    if (this != &other)
    {
        clear();
        size = other.size;
        rear = other.rear;
        other.size = 0;
        other.rear = nullptr;
    }
    return *this;
}

void PriorityPointQueue::swap(PriorityPointQueue &other) noexcept
{
    std::swap(size, other.size);
    std::swap(rear, other.rear);
}

PriorityPointQueue PriorityPointQueue::Build()
{
    PriorityPointQueue cola;

    return cola;
}

Node<Punto> *PriorityPointQueue::getRear()
{
    return rear;
}

void PriorityPointQueue::pop()
{
    removeNode(rear);
}

void PriorityPointQueue::clear()
{
    while (rear != null)
    {
        Node<Punto> *tmp;
        tmp = rear;
        rear = tmp->next;
        delete tmp;
    }
    size = 0;
}

void PriorityPointQueue::enqueue(Punto pt)
{
    Node<Punto> *newNode, *currentNode, *previousNode;

    if (isEmpty())
    {
        rear = new Node<Punto>(pt);
        size++;

        return;
    }

    currentNode = rear;

    while (currentNode != HEAD)
    {
        // Do not add if the point is already in list.
        if (pt == currentNode->data)
        {
            return;
        }
        //  If satisfies condition, it is added at front.
        if (condition(pt, currentNode->data))
        {
            addNodeBeforeAt(currentNode, pt);

            return;
        }
        previousNode = currentNode;
        currentNode = currentNode->next;
    }

    // Other cases, previousNode -> HEAD ==> previousNode <-> newNode -> HEAD
    newNode = new Node<Punto>(pt);
    newNode->previous = previousNode;
    previousNode->next = newNode;

    size++;
}

// ... prevNode  <-> newNode <-> currentNode ...
void PriorityPointQueue::addNodeBeforeAt(Node<Punto> *currentNode, Punto point)
{
    if (!currentNode)
        throw std::invalid_argument("Cannot insert before a null queue node.");

    Node<Punto> *newNode, *previousNode;
    newNode = new Node<Punto>(point);

    // case 1: // REAR-> currentNode ==> REAR -> newNode <-> currentNode
    if (currentNode->previous == REAR)
    {
        newNode->next = currentNode;     // NULL-> newNod -> currentNode && REAR-> currentNode
        currentNode->previous = newNode; // NULL -> newNod <-> currentNode ...
        rear = newNode;                  // REAR -> newNode <-> currentNode ...
    }
    // case 2: ... prevNode <-> currentNode ==> ... prevNode <-> newNode <-> currentNode
    else
    {
        previousNode = currentNode->previous;
        newNode->next = currentNode; // NULL -> newNod -> currentNode && prevNode <-> currentNode
        newNode->previous =
            currentNode->previous;       // prevNode <- newNod -> currentNode && prevNode <-> currentNode
        currentNode->previous = newNode; // prevNode <- newNod <-> currentNode && prevNode -> currentNode
        previousNode->next = newNode;    // prevNode <-> newNod <-> currentNode
    }
    ++size;
}

void PriorityPointQueue::update()
{
    if (rear == REAR)
    {
        return;
    }
    Node<Punto> *node, *prevNode;
    node = rear;

    while (node->next != null)
    {
        prevNode = node;
        node = node->next;
        Punto q = node->data;
        Punto qprev = prevNode->data;
        // A shared coordinate is not a geometric dominance proof.
        if (qprev == q)
        {
            removeNode(node);
            node = prevNode;
        }
    }
}

void PriorityPointQueue::removeNode(Node<Punto> *node)
{
    if (!node)
        return;

    auto *previous = node->previous;
    auto *next = node->next;

    if (previous)
        previous->next = next;
    else
        rear = next;
    if (next)
        next->previous = previous;
    delete node;
    --size;
}

int PriorityPointQueue::getSize()
{
    return size;
}

bool PriorityPointQueue::isEmpty()
{
    return rear == null;
}

bool PriorityPointQueue::condition1(Punto pt1, Punto pt2)
{
    return pt1.x < pt2.x;
}

bool PriorityPointQueue::condition2(Punto pt1, Punto pt2)
{
    return pt1.x == pt2.x && pt1.z < pt2.z;
}

bool PriorityPointQueue::condition3(Punto pt1, Punto pt2)
{
    return pt1.x == pt2.x && pt1.z == pt2.z && pt1.y < pt2.y;
}

bool PriorityPointQueue::condition(Punto pt1, Punto pt2)
{
    return condition1(pt1, pt2) || condition2(pt1, pt2) || condition3(pt1, pt2);
}

std::ostream &operator<<(std::ostream &COUT, PriorityPointQueue &cola)
{
    if (!cola.isEmpty())
    {
        Node<Punto> *rear = cola.getRear();
        COUT << "REAR->";

        while (rear != null)
        {
            COUT << rear->data;

            if (rear->next == null)
            {
                COUT << "->HEAD";
            }
            else
            {
                COUT << "<->";
            }
            rear = rear->next;
        }
        rear = null;
        delete rear;
    }

    return COUT;
}

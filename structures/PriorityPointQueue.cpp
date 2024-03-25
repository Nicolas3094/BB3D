#include "PriorityPointQueue.hpp"

PriorityPointQueue::PriorityPointQueue()
{
    this->size = 0;
    this->rear = null;
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
    if (isEmpty())
    {
        return;
    }
    Node<Punto> *tmp;
    tmp = rear;
    rear = tmp->next;

    delete tmp;
    tmp = null;
}

void PriorityPointQueue::clear()
{
    while (rear != null)
    {
        Node<Punto> *tmp;
        tmp = rear;
        rear = tmp->next;
        delete tmp;
        tmp = null;
    }
    size = 0;
}

void PriorityPointQueue::enqueue(Punto pt)
{
    Node<Punto> *newNode, *currentNode, *previousNode;
    // std::cout << "point: " << pt << "\n";
    if (isEmpty())
    {
        rear = new Node<Punto>(pt);

        currentNode = null;
        newNode = null;
        previousNode = null;
        delete newNode;
        delete currentNode;
        delete previousNode;
        size++;
        return;
    }

    currentNode = rear;
    while (currentNode != HEAD)
    {
        // Do not add if the point is already in list.
        if (pt == currentNode->data)
        {
            currentNode = null;
            newNode = null;
            previousNode = null;
            delete newNode;
            delete currentNode;
            delete previousNode;

            return;
        }
        //  If satisfies condition, it is added at front.
        if (condition(pt, currentNode->data))
        {
            addNodeBeforeAt(currentNode, pt);
            currentNode = null;
            newNode = null;
            previousNode = null;
            delete newNode;
            delete currentNode;
            delete previousNode;
            size++;
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

    currentNode = null;
    newNode = null;
    previousNode = null;
    delete newNode;
    delete currentNode;
    delete previousNode;

    // std::cout << *this << "\n";
}

// ... prevNode  <-> newNode <-> currentNode ...
void PriorityPointQueue::addNodeBeforeAt(Node<Punto> *currentNode, Punto point)
{
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
        newNode->next = currentNode;               // NULL -> newNod -> currentNode && prevNode <-> currentNode
        newNode->previous = currentNode->previous; // prevNode <- newNod -> currentNode && prevNode <-> currentNode
        currentNode->previous = newNode;           // prevNode <- newNod <-> currentNode && prevNode -> currentNode
        previousNode->next = newNode;              // prevNode <-> newNod <-> currentNode
    }

    currentNode = null;
    newNode = null;
    previousNode = null;
    delete newNode;
    delete currentNode;
    delete previousNode;
}
void PriorityPointQueue::update()
{
    Node<Punto> *node, *prevNode;
    if (rear == REAR)
        return;
    node = rear;
    while (node->next != null)
    {
        prevNode = node;
        node = node->next;
        Punto q = node->data;
        Punto qprev = prevNode->data;
        if ((qprev.x == q.x && qprev.z == q.z) || (qprev.x < q.x && qprev.y == q.y && qprev.z == q.z))
        {
            removeNode(node);
            node = prevNode;
        }
    }

    node = null;
    prevNode = null;
    delete node;
    delete prevNode;
}

void PriorityPointQueue::removeNode(Node<Punto> *node)
{
    // lets erase node
    // <-> prevNode <-> node <-> nextNode <->
    Node<Punto> *tmp, *prevNode, *nextNode;
    tmp = node;

    prevNode = node->previous;
    nextNode = node->next;

    // Case: REAR <-> node  <-> HEAD
    if (prevNode == null && nextNode == null)
    {
        rear = null;
        delete rear;
    }
    // Case: REAR -> node <-> nextNode <-> ..
    else if (prevNode == null && nextNode != null)
    {
        // Result: REAR <-> nextNode <->
        nextNode->previous = null;
        rear = nextNode;
    }
    // Case: ... <-> prevNode <-> node <-> HEAD.
    else if (prevNode != null && nextNode == null)
    {
        // Result: ... <-> prevNode <-> HEAD.
        prevNode->next = null;
    }
    // Case: ... <-> prevNode <-> node <-> nextNode <-> ..
    else
    {
        // ... <-> prevNode <-> nextNode <-> ...
        prevNode->next = nextNode;
        nextNode->previous = prevNode;
    }

    size--;

    nextNode = null;
    prevNode = null;
    tmp = null;
    node = null;
    delete node;
    delete nextNode;
    delete tmp;
    delete prevNode;
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
    }

    return COUT;
}

#include "LinkedList.h"

typedef struct nodeCDT {
    void * element;
    nodeADT next, previous;
} nodeCDT;

LinkedList::LinkedList() {
    count = length = 0;
    head = last = NULL;
}

LinkedList::~LinkedList() {
    nodeADT node = head;
    nodeADT aux;
    while (node != NULL) {
        aux = node->next;
        this->remove(node);
        node = aux;
    }
}

nodeADT LinkedList::add(void* element) {
    nodeADT node = calloc(1, sizeof(*node));
    if (node == NULL) return NULL;

    if (head == NULL) {
        head = node;
    } else {
        last->next = node;
        node->previous = last;
    }
    last = node;
    node->element = element;
    
    return node;
}

uint32_t LinkedList::getLength() {
    return count;
}

nodeADT LinkedList::getNode(uint32_t index) {
    if (count <= index) return NULL;
    nodeADT node = head;
    while (index > 0) {
        node = node->next;
    }
    return node;
}

void * LinkedList::getElement(nodeADT node) {
    if (node == NULL) return NULL;
    return node->element
}

void LinkedList::removeNode(nodeADT node) {
    if (node == NULL) return NULL;
    this->_removeNode(node);
}

void LinkedList::remove(uint32_t index) {
    this->removeNode(this->getNode(index));
}

// Private
void LinkedList::_removeNode(nodeADT node) {
    if (node->previous != NULL) {
        node->previous->next = node->next;
    } else {
        head = node->next;
    }

    if (node->next != NULL) {
        node->next->previous = node->previous;
    } else {
        last = node->previous;
    }
    free(node);
}

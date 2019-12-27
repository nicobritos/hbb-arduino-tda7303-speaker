#ifndef _LINKED_LIST_H

    #include "Arduino.h"

    #define _LINKED_LIST_H

    typedef struct nodeCDT *nodeADT;

    class LinkedList {
    private:
        uint32_t count;
        uint32_t length;
        nodeADT head, last, currentIterator;

        void LinkedList::_removeNode(nodeADT node);

    public:
        LinkedList();
        ~LinkedList();
        
        nodeADT add(void* element);
        uint32_t getLength();
        nodeADT getNode(uint32_t index);
        void * getElement(nodeADT node);
        void removeNode(nodeADT node);
        void remove(uint32_t index);

        // Iterator
        void setIterator();
        uint8_t hasNext();
        nodeADT next();
    };

#endif

#ifndef GENERIC_SLL_H
#define GENERIC_SLL_H

#include <stddef.h>

typedef struct node {
    void *data;
    struct node *next;
} Node;

typedef struct sll {
    Node *head;
    Node *tail;
    size_t size;
    void (*freeData)(void*);
} Sll;

// Memory
void* safeMalloc(size_t size);
void freeData(void* data);

// List operations
Sll* createSll(void (*free_func)(void*));
Node* createNode(void *data);
void pushFront(Sll *s, void *data);
void pushBack(Sll *s, void *data);
void* popFront(Sll *s);
void* popBack(Sll *s);
void printSll(Sll *s, void (*custom_printf)(void *));
void freeSll(Sll *s);
int findNodeIndex(Sll *s, Node *node);
void* peekAtIndex(Sll *s, int index);
void deleteAtIndex(Sll *s, int index);
void deleteValue(Sll *s, void *value);
void bubbleSort(Sll *s, int (*cmp)(const void *, const void *));

#endif // GENERIC_SLL_H
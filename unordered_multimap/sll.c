#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    void *data;
    struct node *next;
}Node;

typedef struct sll {
    Node *head;
    Node *tail;
    size_t size;
    void (*freeData)(void*);
}Sll;

void* safeMalloc(size_t size) {
    void *p = malloc(size);

    if (p == NULL) {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    return p;
}

void freeData(void* data) {
    // this can receive custom logic depending on the data info
    // this also assumes that each node in the list has the same type
    // and will not work with different types in the same list
    free(data);
}

Sll* createSll(void (*free_func)(void*)) {
    Sll *s = (Sll*)safeMalloc(sizeof(Sll));
    s->head = NULL;
    s->tail = NULL;
    s->freeData = free_func;
    s->size = 0;

    return s;
}

Node* createNode(void *data) {
    Node *node = (Node*)safeMalloc(sizeof(Node));
    node->data = data;
    node->next = NULL;

    return node;
}

void pushFront(Sll *s, void *data) {
    if (s == NULL) {
        printf("Empty list\n");
        return;
    }

    Node *node = createNode(data);

    node->next = s->head;
    s->head = node;
    s->size++;

    if (s->tail == NULL) {
        s->tail = node;
    }
}

void pushBack(Sll *s, void *data) {
    if (s == NULL) {
        printf("Empty list\n");
        return;
    }

    Node *node = createNode(data);

    if (s->head == NULL) {
        s->head = node;
        s->tail = node;
    }
    else {
        s->tail->next = node;
        s->tail = node;
    }

    s->size++;
}

void* popFront(Sll *s) {
    if (s == NULL || s->head == NULL) {
        printf("Empty list\n");
        return NULL;
    }

    Node *node = s->head;
    void *data = node->data;
    s->head = node->next;

    if (s->head == NULL)
        s->tail = NULL;

    free(node);
    s->size--;

    return data;
}

void* popBack(Sll *s) {
    if (s == NULL || s->tail == NULL) {
        printf("Empty list\n");
        return NULL;
    }

    void *data;
    if (s->head == s->tail) {
        data = s->head->data;
        free(s->head);

        s->head = NULL;
        s->tail = NULL;
        s->size = 0;

        return data;
    }

    Node *current = s->head;
    while (current->next != s->tail) {
        current = current->next;
    }

    data = s->tail->data;
    free(s->tail);

    current->next = NULL;
    s->tail = current;
    s->size--;

    return data;
}

int findNodeIndex(Sll *s, Node *node) {
    if (s == NULL || node == NULL) {
        printf("Empty node or list\n");
        return -1;
    }

    int counter;
    Node *current;
    for (current = s->head, counter = 0; current != NULL; current = current->next, counter++) {
        if (current == node) {
            return counter;
        }
    }

    return -1;
}

void* peekAtIndex(Sll *s, int index) {
    if (s == NULL || index < 0 || s->size <= index) {
        printf("Invalid index or empty list\n");
        return NULL;
    }

    Node *current;
    int counter;

    for (current = s->head, counter = 0; current != NULL; current = current->next, counter++) {
        if (counter == index) {
            return current->data;
        }
    }

    return NULL;
}

void deleteAtIndex(Sll *s, int index) {
    if (s == NULL || index < 0 || s->size <= index) {
        printf("Invalid index or empty list\n");
        return;
    }

    if (index == 0) {
        void *data = popFront(s);
        if (data) s->freeData(data);
        return;
    }

    if (index == s->size - 1) {
        void *data = popBack(s);
        if (data) s->freeData(data);
        return;
    }

    Node *current = s->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    Node *to_delete = current->next;
    current->next = to_delete->next;

    s->freeData(to_delete->data);
    free(to_delete);
    s->size--;
}

void deleteValue(Sll *s, void *value) {
    if (s == NULL) {
        printf("Empty list\n");
        return;
    }

    int counter;
    Node *current;
    for (current = s->head, counter = 0; current != NULL; current = current->next, counter++) {
        if (current->data == value) {
            deleteAtIndex(s, counter);
            return;
        }
    }
}

void bubbleSort(Sll *s, int (*cmp)(const void *, const void *)) {
    if (s == NULL || s->size == 0) {
        printf("Empty list\n");
        return;
    }

    for (Node* i = s->head; i != NULL; i = i->next) {
        for (Node* j = i->next; j != NULL; j = j->next) {
            if (cmp(i->data, j->data)) {
                void *tmp = i->data;
                i->data = j->data;
                j->data = tmp;
            }
        }
    }
}

void printSll(Sll *s, void (*customPrint)(void *)) {
    if (s == NULL) {
        printf("Empty list\n");
        return;
    }

    for (Node *i = s->head; i != NULL; i = i->next) {
        customPrint(i->data);
    }
}

void freeSll(Sll *s) {
    if (s == NULL) {
        printf("List is already empty\n");
        return;
    }

    if (s->size == 0) {
        free(s);
        return;
    }

    Node *current = s->head;
    while (current != NULL) {
        Node *next = current->next;
        s->freeData(current->data);
        free(current);
        current = next;
    }

    free(s);
    s = NULL;

    printf("List is freed\n");
}
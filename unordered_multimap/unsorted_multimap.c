#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic_sll.h"

typedef struct multimap {
    Sll **map;
    int mapSize;
    int sllSize;
}Multimap;

int hashValue(const int value, const int size) {
    return value % size;
}

Sll** createMultimap(const int size) {
    if (size <= 0) {
        return NULL;
    }

    Sll** arr = (Sll**)safeMalloc(size * sizeof(Sll*));
    for (int i = 0; i < size; i++) {
        arr[i] = createSll(freeData);
    }

    return arr;
}

void freeMultimap(Multimap *map) {
    if (map == NULL || map->map == NULL) {
        printf("Multimap is empty");
        return;
    }

    for (int i = 0; i < map->mapSize; i++) {
        freeSll(map->map[i]);
    }
    free(map->map);
}

void safeDoubleSize(Multimap *map) {
    int oldSize = map->mapSize;
    int newSize = oldSize * 2;

    Sll **newMap = createMultimap(newSize);

    for (int i = 0; i < oldSize; i++) {
        Sll *list = map->map[i];
        if (!list || list->head == NULL) continue;

        for (Node *n = list->head; n != NULL; n = n->next) {
            int value = *(int *)n->data;
            int index = hashValue(value, newSize);
            int *newVal = (int *)safeMalloc(sizeof(int));
            *newVal = value;
            pushBack(newMap[index], newVal);
        }
        freeSll(list);
    }

    free(map->map);
    map->map = newMap;
    map->mapSize = newSize;
}

void addToMultimap(Multimap *map, const int value) {
    if (map->map == NULL) {
        printf("Multimap is empty");
        return;
    }

    int index = hashValue(value, map->mapSize);
    int *temp = (int*)safeMalloc(sizeof(int));
    *temp = value;

    Sll *list = map->map[index];
    if (list->size < map->sllSize) {
        pushBack(list, temp);
    }
    else {
        safeDoubleSize(map);
        int newIndex = hashValue(value, map->mapSize);
        pushBack(map->map[newIndex], temp);
    }
}

void removeFromMultimap(Multimap *map, const int key, const int value) {
    if (map->map == NULL) {
        printf("Multimap is empty");
        return;
    }

    int index = hashValue(key, map->mapSize);

    if (map->map[index]->head == NULL) {
        printf("Multimap is empty");
        return;
    }

    void *peek;
    for (int i = 0; i < (int)map->map[index]->size; i++) {
        peek = peekAtIndex(map->map[index], i);
        if (peek != NULL && *(int*)peek == value) {
            deleteValue(map->map[index], peek);
            return;
        }
    }
}

void removeAllKeys(Multimap *map, const int key) {
    if (map->map == NULL) {
        printf("Multimap is empty");
        return;
    }

    int index = hashValue(key, map->mapSize);

    freeSll(map->map[index]);
    map->map[index] = createSll(freeData);
}

Sll *findKey(Multimap *map, const int key) {
    if (map->map == NULL) {
        printf("Multimap is empty");
        return NULL;
    }

    int index = hashValue(key, map->mapSize);
    return map->map[index];
}

int containsKey(Multimap *map, const int key) {
    if (map->map == NULL) {
        printf("Multimap is empty");
        return 0;
    }

    int index = hashValue(key, map->mapSize);
    if (map->map[index]->head == NULL) {
        return 0;
    }

    return 1;
}

int containsValue(Multimap *map, const int key, const int value) {
    if (containsKey(map, key)) {
        Sll *aux = findKey(map, key);
        void *peek;

        for (int i = 0; i < map->sllSize; i++) {
            peek = peekAtIndex(aux, i);
            if (peek && *(int*)peek == value) {
                return 1;
            }
        }
    }

    return 0;
}

void printMultimap(Multimap *map) {
    if (map->map == NULL) {
        printf("Empty map or invalid size");
        return;
    }

    for (int i = 0; i < map->mapSize; i++) {
        if (map->map[i]->head != NULL) {
            printf("\n----INDEX %d----\n", i);

            for (Node *j = map->map[i]->head; j != NULL; j = j->next) {
                printf("%d ", *(int*)j->data);
            }
        }
    }
    printf("\n");
}
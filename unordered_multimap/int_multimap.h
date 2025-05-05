#ifndef MULTIMAP_H
#define MULTIMAP_H

#include "generic_sll.h"

typedef struct multimap {
    Sll **map;
    int mapSize;
    int sllSize;
} Multimap;

// Core functions
void safeDoubleSize(Multimap *map);
void rehashValues(Multimap map);
Sll **createMultimap(const int size);
void freeMultimap(Multimap *map);

// Basic operations
void addToMultimap(Multimap *map, const int value);
void removeFromMultimap(Multimap *map, const int key, const int value);
void removeAllKeys(Multimap *map, const int key);
Sll *findKey(Multimap *map, const int key);

// Query functions
int containsKey(Multimap *map, const int key);
int containsValue(Multimap *map, const int key, const int value);

// Utility
void printMultimap(Multimap *map);
int hashValue(const int value, const int size);

#endif // MULTIMAP_H

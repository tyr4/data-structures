#pragma once

#ifndef BTREE_INTERNAL_H
#define BTREE_INTERNAL_H

#include <stddef.h>
#include "btree.h"

#define max(a, b) ((a) > (b)? (a) : (b))

typedef struct node {
    int *keys;
    struct node **children;
    struct node *parent;
    int currentKeys;
    int isLeaf;
} BTreeNode;

typedef struct btree {
    BTreeNode *root;
    int t;
    int maxKeys;
    int minKeys;
} BTree;

typedef struct splitResult {
    int hasSplit;
    int promotedKey;
    BTreeNode *rightChild;
} SplitResult;

// internal helpers
// memory initialization
void* safeMalloc(size_t size);
BTreeNode* initRoot(int maxKeys);
BTreeNode* initChild(BTreeNode *parent, int maxKeys);

// utils
int getNewKeyPosition(int* array, int size, int key);
int hasKeyInNode(int* array, int size, int key);
void insertSortedArray(int* array, int *size, int key);
void deleteFromArray(int* array, int *size, int pos);

// insertion helpers
void splitChild(BTreeNode* parent, int childIndex, int maxKeys);
SplitResult insertHelper(BTreeNode* root, int key, int maxKeys);

// deletion helpers
BTreeNode* getChild(BTreeNode* root, int key);
BTreeNode* getRightmostChild(BTreeNode* root);

void borrowFromLeft(BTreeNode* parent, int childIndex);
void borrowFromRight(BTreeNode* parent, int childIndex);
int mergeChildren(BTreeNode* parent, int childIndex, int minKeys);
int fixUnderflow(BTreeNode* parent, int childIndex, int minKeys);

int deleteHelper(BTreeNode* root, int key, int minKeys);

// misc helpers
void freeBTreeHelper(BTreeNode* root);
void printBTreeHelper(BTreeNode* root);

#endif
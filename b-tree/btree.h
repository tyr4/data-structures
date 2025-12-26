#pragma once

#ifndef DATA_STRUCTURES_BTREE_H
#define DATA_STRUCTURES_BTREE_H

// data structures
typedef struct node BTreeNode;
typedef struct btree BTree;

// public API for main
BTree* initBTree(int t);

void insertToBTreeTopToBottom(BTree *tree, int key);
void insertToBTreeBottomToTop(BTree *tree, int key);

void deleteFromBTree(BTree *tree, int key);

void freeBTree(BTree *tree);
void printBTree(BTree* tree);

#endif
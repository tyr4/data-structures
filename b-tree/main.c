#include <stdio.h>
#include <stdlib.h>

#define MAX_NODES 4

typedef struct node {
    int *keys;
    struct node **children;
    struct node *parent;

    int leaf;
    int currentNodes;
} BTreeNode;

typedef struct btree {
    BTreeNode *root;

    int t; // minimum degree
} BTree;

void* safeMalloc(size_t size) {
    void* p = malloc(size);

    if (p == NULL) {
        perror("malloc");
        exit(1);
    }

    return p;
}

BTree* initBTree() {
    BTree *tree = (BTree*)safeMalloc(sizeof(BTree));
    tree->root = (BTreeNode*)safeMalloc(sizeof(BTreeNode));
    tree->root->parent = (BTreeNode*)safeMalloc(sizeof(BTreeNode));
    tree->root->keys = (int*)safeMalloc(sizeof(int) * MAX_NODES);
    tree->root->children = (BTreeNode**)safeMalloc(sizeof(BTreeNode*) * (MAX_NODES + 1));

    tree->t = MAX_NODES / 2 + 1;
    tree->root->currentNodes = 0;
    tree->root->leaf = 0;

    return tree;
}

void freeBTree(BTreeNode* root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i <= root->currentNodes; i++) {
        freeBTree(root->children[i]);
    }

    free(root);
}


int main(void) {
    BTree* tree = initBTree();

    freeBTree(tree->root);
    free(tree);

    return 0;
}

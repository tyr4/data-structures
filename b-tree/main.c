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

BTreeNode* initNode() {
    BTreeNode *node = (BTreeNode*)safeMalloc(sizeof(BTreeNode));
    node->keys = (int*)safeMalloc(sizeof(int) * MAX_NODES);
    node->children = (BTreeNode**)safeMalloc(sizeof(BTreeNode*) * (MAX_NODES + 1));
    node->parent = NULL;
    node->currentNodes = 0;
    node->leaf = 1;

    // make sure the children are set on NULL
    for (int i = 0; i <= MAX_NODES; i++) {
        node->children[i] = NULL;
    }

    return node;
}

BTree* initBTree() {
    BTree *tree = (BTree*)safeMalloc(sizeof(BTree));
    tree->root = initNode();
    tree->t = MAX_NODES / 2 + 1;

    return tree;
}

void freeBTree(BTreeNode* root) {
    if (root == NULL) {
        return;
    }

    // traverse all children, root->curr + 1
    for (int i = 0; i <= root->currentNodes; i++) {
        if (root->children[i] != NULL) {
            freeBTree(root->children[i]);
        }
    }

    free(root->keys);
    free(root->children);
    free(root);
}

void insertToBTree(BTreeNode* root, int key) {
    if (root->leaf) {
        // insert sorted
        for (int i = 0; i < root->currentNodes; i++) {
            if (root->keys[i] > key) {
                // check if the root can store more nodes before being split
                if (root->currentNodes != MAX_NODES) {
                    int aux = root->keys[i];
                    root->keys[i] = key;
                    root->keys[i + 1] = aux;
                    return;
                }
            }
        }
    }
}

// void printBTree(BTreeNode* root) {
//     if (root == NULL) {
//         return;
//     }
//
//     for (int i = 0; i <= root->currentNodes; i++) {
//         printBTree(root->children[i]);
//         printf("%d", root->keys[i]);
//     }
// }

int main(void) {
    BTree* tree = initBTree();

    tree->root->keys[0] = 1;
    tree->root->keys[1] = 2;
    tree->root->keys[2] = 3;
    tree->root->keys[3] = 4;
    tree->root->currentNodes = 4;

    tree->root->children[0] = initNode();

    tree->root->children[0]->keys[0] = 1;
    tree->root->children[0]->keys[1] = 2;
    tree->root->children[0]->keys[2] = 3;
    tree->root->children[0]->keys[3] = 4;
    tree->root->children[0]->currentNodes = 4;

    // printBTree(tree->root);
    freeBTree(tree->root);
    free(tree);

    return 0;
}

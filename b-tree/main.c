#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYS 4

typedef struct node {
    int *keys;
    struct node **children;
    struct node *parent;

    int leaf;
    int currentKeys;
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

BTreeNode* initRoot() {
    BTreeNode *node = (BTreeNode*)safeMalloc(sizeof(BTreeNode));

    node->keys = (int*)safeMalloc(sizeof(int) * MAX_KEYS);
    node->children = (BTreeNode**)safeMalloc(sizeof(BTreeNode*) * (MAX_KEYS + 1));
    node->parent = NULL;
    node->currentKeys = 0;
    node->leaf = 1;

    // make sure the children are set on NULL and keys on -1
    for (int i = 0; i <= MAX_KEYS; i++) {
        node->children[i] = NULL;

        if (i < MAX_KEYS) node->keys[i] = 0;
    }


    return node;
}

BTreeNode* initChild(BTreeNode *parent) {
    BTreeNode *node = initRoot();

    // inainte sa fac node->parent = parent, faceam node->parent = malloc(node) si ii dadeam assign dupa
    node->parent = parent;
    parent->leaf = 0;

    return node;
}

BTree* initBTree() {
    BTree *tree = (BTree*)safeMalloc(sizeof(BTree));
    tree->root = initRoot();
    tree->t = MAX_KEYS / 2 + 1;

    return tree;
}

void freeBTree(BTreeNode *root) {
    if (root == NULL) {
        return;
    }

    // traverse all children, root->curr + 1
    for (int i = 0; i <= root->currentKeys; i++) {
        if (root->children[i] != NULL) {
            freeBTree(root->children[i]);
        }
    }

    // daca bagam linia asta aici crapa la urmatorul check in forul de mai sus
    // if (root->parent != NULL) free(root->parent);
    free(root->keys);
    free(root->children);
    free(root);
}

// BTreeNode* splitChild(BTreeNode *node) {
//     BTreeNode *left = initRoot();
//     BTreeNode *right = initRoot();
// }

int getKeyPosition(int *array, int size, int key) {
    if (size == 0) return 0;

    int pos = 0;

    if (key < array[0]) pos = 0;                    // left
    else if (key > array[size - 1]) pos = size;     // right
    else for (int i = 0; i < size - 1; i++) {       // inbetween
        if (key > array[i] && key < array[i + 1]) {
            pos = i + 1;
            break;
        }
    }

    return pos;
}

void insertSortedArray(int *array, int *size, int key) {
    if (*size == 0) {
        array[(*size)++] = key;
        return;
    }

    int pos = getKeyPosition(array, *size, key);

    // move the nodes 1 position to the right starting from pos
    for (int i = *size; i >= pos; i--) {
        array[i] = array[i - 1];
    }

    array[pos] = key;
    (*size)++;
}

void insertToBTree(BTree* tree, BTreeNode* root, int key) {
    if (root == NULL) {
        return;
    }

    // find the right leaf
    if (!root->leaf) {
        int keyPos = getKeyPosition(root->keys, root->currentKeys, key);

        // key goes to the leftmost child
        if (keyPos == 0) {
            insertToBTree(tree, root->children[0], key);
        }

        // key goes to the rightmost child
        else if (keyPos == root->currentKeys) {
            insertToBTree(tree, root->children[root->currentKeys], key);
        }

        // key goes inbetween
        else {
            insertToBTree(tree, root->children[keyPos + 1], key);
        }
    }

    else {
        // check if the node has space
        if (root->currentKeys < MAX_KEYS) {
            insertSortedArray(root->keys, &root->currentKeys, key);
        }
        else {
            printf("Couldnt add %d because the node is full\n", key);
            // check if the root has to be split
            if (tree->root == root) {
                // get the middle key
                int temp[root->currentKeys + 1], tempSize = root->currentKeys;
                memcpy(temp, root->keys, root->currentKeys * sizeof(int));
                insertSortedArray(temp, &tempSize, key);
                int middleKey = temp[tempSize / 2];

                // make a new root node
                BTreeNode *newRoot = initRoot();

                // insert the key to it
                newRoot->keys[0] = middleKey;
                newRoot->currentKeys++;

                // make the left and right children
                BTreeNode *leftChild = initChild(newRoot);
                BTreeNode *rightChild = initChild(newRoot);

                // append to left child the left side
                memcpy(leftChild->keys, temp, (tempSize / 2) * sizeof(int));
                memcpy(leftChild->children, root->children, (tempSize / 2 + 1) * sizeof(BTreeNode*));
                leftChild->currentKeys = tempSize / 2;

                // same to right child
                memcpy(rightChild->keys, temp + tempSize / 2 + 1, (tempSize / 2) * sizeof(int));
                memcpy(rightChild->children, root->children + tempSize / 2 + 1, (tempSize / 2) * sizeof(BTreeNode*));
                rightChild->currentKeys = tempSize / 2;

                // edit the new roots children
                newRoot->children[0] = leftChild;
                newRoot->children[1] = rightChild;
                tree->root = newRoot;

                // the current root is unused now
                free(root->keys);
                free(root->children);
                free(root);
            }

        }
    }
}

void printBTree(BTreeNode* root) {
    if (root == NULL) {
        return;
    }

    // first print the roots keys
    printf("Root: \t\t");
    for (int i = 0; i < root->currentKeys; i++) {
        printf("%d ", root->keys[i]);
    }

    printf("\n");

    // loop through the roots children
    for (int child = 0; child <= root->currentKeys; child++) {
        printf("Child %d:\t", child);

        // check if the child doesnt exist
        if (root->children[child] == NULL) {
            printf("NULL");
            printf("\n");
            continue;
        }

        // print each childs keys
        for (int i = 0; i < root->children[child]->currentKeys; i++) {
            printf("%d ", root->children[child]->keys[i]);
        }

        printf("\n");

    }
    printf("\n");

    // recurse through the roots children
    for (int i = 0; i <= root->currentKeys; i++) {
        printBTree(root->children[i]);
    }
}

int main(void) {
    BTree* tree = initBTree();

    insertToBTree(tree, tree->root, 10);
    insertToBTree(tree, tree->root, 20);
    insertToBTree(tree, tree->root, 30);
    insertToBTree(tree, tree->root, 40);
    insertToBTree(tree, tree->root, 50);

    printBTree(tree->root);

    freeBTree(tree->root);
    free(tree);

    return 0;
}

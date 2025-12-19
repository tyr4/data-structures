#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int treeDegree = 3;
#define MAX_KEYS (2 * treeDegree - 1)
#define MINIMUM_KEYS (treeDegree - 1)

typedef struct node {
    int *keys;
    struct node **children;
    struct node *parent;

    int currentKeys;
} BTreeNode;

typedef struct btree {
    BTreeNode *root;

    int t; // minimum degree
} BTree;

typedef struct splitResult {
    int hasSplit;
    int promotedKey;
    BTreeNode *rightNode;
} SplitResult;

// need to declare the header for merging
void fixUnderflow(BTreeNode*, int);

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

    // make sure the children are set on NULL and keys on -1
    for (int i = 0; i < MAX_KEYS; i++) {
        node->children[i] = NULL;
        node->keys[i] = 0;
    }
    node->children[MAX_KEYS] = NULL;

    return node;
}

BTreeNode* initChild(BTreeNode *parent) {
    BTreeNode *node = initRoot();

    node->parent = parent;

    return node;
}

BTree* initBTree(int t) {
    BTree *tree = (BTree*)safeMalloc(sizeof(BTree));
    tree->root = initRoot();
    tree->t = t;

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

    free(root->keys);
    free(root->children);
    free(root);
}

int isLeaf(BTreeNode *node) {
    for (int i = 0; i <= node->currentKeys; i++) {
        if (node->children[i] != NULL) {
            return 0;
        }
    }

    return 1;
}

int getKeyPosition(int *array, int size, int key) {
    if (size == 0) return 0;

    int pos = 0;

    // the = stuff is for calls from the delete functions
    if (key <= array[0]) pos = 0;                    // left
    else if (key >= array[size - 1]) pos = size;     // right
    else for (int i = 0; i < size - 1; i++) {       // inbetween
        if (key >= array[i] && key <= array[i + 1]) {
            pos = i + 1;
            break;
        }
    }

    return pos;
}
// funni binary search
int hasKeyInNode(int *array, int size, int key) {
    int left = 0, right = size - 1;

    while (left <= right) {
        int mid = (left + right) / 2;

        if (key == array[mid]) {
            return 1;
        }

        if (key > array[mid]) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return array[left] == key;
}

void insertSortedArray(int *array, int *size, int key) {
    if (*size == 0) {
        array[(*size)++] = key;
        return;
    }

    int pos = getKeyPosition(array, *size, key);

    // move the nodes 1 position to the right starting from pos
    for (int i = *size; i >= pos && i > 0; i--) {
        array[i] = array[i - 1];
    }

    array[pos] = key;
    (*size)++;
}

void deleteFromArray(int *array, int *size, int pos) {
    for (int i = pos; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }

    (*size)--;
}

SplitResult insertInternal(BTreeNode* root, int key) {
    SplitResult result = {0, 0, NULL};

    // case 1: leaf
    if (isLeaf(root)) {
        if (root->currentKeys < MAX_KEYS) {
            insertSortedArray(root->keys, &root->currentKeys, key);
            return result;
        }

        // case 1A: leaf split
        // simulate adding the key to the key array
        int temp[root->currentKeys + 1], tempSize = root->currentKeys;
        memcpy(temp, root->keys, root->currentKeys * sizeof(int));
        insertSortedArray(temp, &tempSize, key);
        int middleKey = temp[tempSize / 2];

        // set the SplitResult data
        result.promotedKey = middleKey;
        result.hasSplit = 1;

        // make a split node
        BTreeNode *rightNode = initChild(root);
        result.rightNode = rightNode;

        // replace the current node with the left side
        int mid = tempSize / 2;
        root->currentKeys = 0;
        for (int i = 0; i < mid; i++) {
            root->keys[root->currentKeys++] = temp[i];
        }

        // set the right childs keys
        for (int i = mid + 1; i < tempSize; i++) {
            rightNode->keys[rightNode->currentKeys++] = temp[i];
        }

        // return the structure with the split data
        return result;
    }

    // case 2: it has propagated to an internal node
    // find the correct child top to bottom
    int pos = getKeyPosition(root->keys, root->currentKeys, key);
    BTreeNode *child = root->children[pos];

    SplitResult childSplit = insertInternal(child, key);

    // the node has been inserted without splits
    if (!childSplit.hasSplit) {
        return result;
    }

    // case 2A: child split, safe to add directly here
    if (root->currentKeys < MAX_KEYS) {
        insertSortedArray(root->keys, &root->currentKeys, childSplit.promotedKey);

        // shift children to the correct place
        for (int i = root->currentKeys; i > pos + 1; i--) {
            root->children[i] = root->children[i - 1];
        }

        root->children[pos + 1] = childSplit.rightNode;
        childSplit.rightNode->parent = root;

        return result;
    }

    // case 2B: a key has propagated here and the node is full
    int tempKeys[root->currentKeys + 1];
    BTreeNode *tempChildren[root->currentKeys + 2];

    // copy the current keys and children
    memcpy(tempKeys, root->keys, root->currentKeys * sizeof(int));
    memcpy(tempChildren, root->children, (root->currentKeys + 1) * sizeof(BTreeNode*));

    // add the new key to the temp array
    int tempKeyCount = root->currentKeys;
    insertSortedArray(tempKeys, &tempKeyCount, childSplit.promotedKey);

    // fix children
    for (int i = tempKeyCount; i > pos + 1; i--) {
        tempChildren[i] = tempChildren[i - 1];
    }

    // split this node
    tempChildren[pos + 1] = childSplit.rightNode;

    int mid = tempKeyCount / 2;
    result.promotedKey = tempKeys[mid];

    result.hasSplit = 1;

    BTreeNode *rightNode = initChild(root->parent);
    result.rightNode = rightNode;

    // replace the left childs data
    root->currentKeys = 0;
    for (int i = 0; i < mid; i++) {
        root->keys[root->currentKeys] = tempKeys[i];
        root->children[root->currentKeys] = tempChildren[i];

        if (tempChildren[i])
            tempChildren[i]->parent = root;

        root->currentKeys++;
    }
    root->children[root->currentKeys] = tempChildren[mid];
    if (tempChildren[mid])
        tempChildren[mid]->parent = root;

    // same for the right node
    for (int i = mid + 1; i < tempKeyCount; i++) {
        rightNode->keys[rightNode->currentKeys] = tempKeys[i];
        rightNode->children[rightNode->currentKeys] = tempChildren[i];

        if (tempChildren[i])
            tempChildren[i]->parent = rightNode;

        rightNode->currentKeys++;
    }
    rightNode->children[rightNode->currentKeys] = tempChildren[tempKeyCount];
    if (tempChildren[tempKeyCount])
        tempChildren[tempKeyCount]->parent = rightNode;

    return result;
}

void insertToBTree(BTree* tree, BTreeNode* root, int key) {
    SplitResult result = insertInternal(root, key);

    if (result.hasSplit) {
        BTreeNode *newRoot = initRoot();
        newRoot->keys[newRoot->currentKeys++] = result.promotedKey;
        newRoot->children[0] = tree->root;
        newRoot->children[1] = result.rightNode;

        result.rightNode->parent = newRoot;
        tree->root->parent = newRoot;
        tree->root = newRoot;
    }
}

BTreeNode* getChild(BTreeNode* root, int key) {
    if (root == NULL) {
        return NULL;
    }

    // check if it has found the child
    if (hasKeyInNode(root->keys, root->currentKeys, key)) {
        return root;
    }

    // not found
    if (isLeaf(root)) {
        return NULL;
    }

    // key is in a child
    // get right child position
    int pos = getKeyPosition(root->keys, root->currentKeys, key);
    return getChild(root->children[pos], key);
}

void borrowFromLeft(BTreeNode* parent, int childIndex) {
    printf("hello from borrow left\n");
    BTreeNode *child = parent->children[childIndex];
    BTreeNode *leftChild = parent->children[childIndex - 1];

    // shift keys to right
    for (int i = child->currentKeys; i > 0; i--) {
        child->keys[i] = child->keys[i - 1];
    }

    child->keys[0] = parent->keys[childIndex - 1];
    child->currentKeys++;

    parent->keys[childIndex - 1] = leftChild->keys[leftChild->currentKeys - 1];
    leftChild->currentKeys--;

    // shift children to right
    for (int i = child->currentKeys; i > 0; i--) {
        child->children[i] = child->children[i - 1];
    }

    child->children[0] = leftChild->children[leftChild->currentKeys + 1];
    leftChild->children[leftChild->currentKeys + 1] = NULL;
}

void borrowFromRight(BTreeNode* parent, int childIndex) {
    printf("hello from borrow right\n");
    BTreeNode *child = parent->children[childIndex];
    BTreeNode *rightChild = parent->children[childIndex + 1];

    child->keys[child->currentKeys++] = parent->keys[childIndex];
    parent->keys[childIndex] = rightChild->keys[0];

    // shift keys to left
    for (int i = 0; i < rightChild->currentKeys - 1; i++) {
        rightChild->keys[i] = rightChild->keys[i + 1];
    }

    rightChild->currentKeys--;

    // TODO: child stuff like borrow left
}

void mergeChildren(BTreeNode* parent, int childIndex) {
    // root case
    if (parent->parent == NULL) {

    }

    BTreeNode *underflowingChild = parent->children[childIndex];

    // check if it can merge with left child
    if (childIndex > 0) {
        BTreeNode *leftChild = parent->children[childIndex - 1];
        int parentSeparator = parent->keys[childIndex - 1];
        int base = leftChild->currentKeys + 1;

        // absorb parent separator
        leftChild->keys[leftChild->currentKeys++] = parentSeparator;
        deleteFromArray(parent->keys, &parent->currentKeys, childIndex - 1);

        // absorb the underflowing child
        for (int i = 0; i < underflowingChild->currentKeys; i++) {
            leftChild->keys[leftChild->currentKeys++] = underflowingChild->keys[i];
        }

        // fix children in case the underflowing child has any
        for (int i = 0; i <= underflowingChild->currentKeys; i++) {
            leftChild->children[base + i] = underflowingChild->children[i];

            // make the parent of the underflowing node the left child instead of the
            // soon-to-be deleted underflowing child
            if (leftChild->children[base + i] != NULL) {
                leftChild->children[base + i]->parent = leftChild;
            }
        }

        // delete the childs link to the parent
        for (int i = childIndex; i <= parent->currentKeys; i++) {
            parent->children[i] = parent->children[i + 1];
        }

        // kil the underflowing child
        // but dont kil its actual children
        free(underflowingChild->keys);
        free(underflowingChild->children);
        free(underflowingChild);
    }

    // we mergin with the right child
    else {
        BTreeNode *rightChild = parent->children[childIndex + 1];
        int parentSeparator = parent->keys[childIndex];
        int oldKeys = underflowingChild->currentKeys;

        // absorb the parent separator
        underflowingChild->keys[underflowingChild->currentKeys++] = parentSeparator;
        deleteFromArray(parent->keys, &parent->currentKeys, childIndex);

        // absorb the right child
        for (int i = 0; i < rightChild->currentKeys; i++) {
            underflowingChild->keys[underflowingChild->currentKeys++] = rightChild->keys[i];
        }

        // fix children
        int base = oldKeys + 1;
        for (int i = 0; i <= rightChild->currentKeys; i++) {
            underflowingChild->children[base + i] = rightChild->children[i];

            if (rightChild->children[i] != NULL) {
                rightChild->children[i]->parent = underflowingChild;
            }
        }

        // delete the childs link to the parent
        for (int i = childIndex + 1; i <= parent->currentKeys; i++) {
            parent->children[i] = parent->children[i + 1];
        }

        // kil the right child
        free(rightChild->keys);
        free(rightChild->children);
        free(rightChild);
    }

    // now check if parent is underflowing
    if (parent->currentKeys < MINIMUM_KEYS) {
        if (parent->parent != NULL) {
            fixUnderflow(parent->parent, getKeyPosition(parent->parent->keys, parent->parent->currentKeys, parent->keys[0]));
        }
    }
}

void fixUnderflow(BTreeNode* parent, int childIndex) {
    // first check if it can borrow from left
    if (childIndex > 0) {
        if (parent->children[childIndex - 1] != NULL && parent->children[childIndex - 1]->currentKeys > MINIMUM_KEYS) {
            borrowFromLeft(parent, childIndex);
            return;
        }
    }

    // then from right
    if (childIndex < parent->currentKeys) {
        if (parent->children[childIndex + 1] != NULL && parent->children[childIndex + 1]->currentKeys > MINIMUM_KEYS) {
            borrowFromRight(parent, childIndex);
            return;
        }
    }

    // forced to merge
    mergeChildren(parent, childIndex);
}

void deleteInternal(BTreeNode* root, int key) {
    // key doesnt exist in tree
    if (root == NULL) {
        return;
    }

    int pos = getKeyPosition(root->keys, root->currentKeys, key);
    // leaf delete operation
    if (isLeaf(root)) {
        int childIndex = getKeyPosition(root->parent->keys, root->parent->currentKeys, key);
        deleteFromArray(root->keys, &root->currentKeys, pos);

        // call helper function if the tree becomes invalid after delete
        if (root->currentKeys < MINIMUM_KEYS) {
            fixUnderflow(root->parent, childIndex);
        }
    }
}

// note that this doesnt work with duplicates
void deleteFromBTree(BTree* tree, int key) {
    BTreeNode *child = getChild(tree->root, key);

    deleteInternal(child, key);
}

void printBTree(BTreeNode* root) {
    if (root == NULL) {
        return;
    }

    // first print the roots keys
    printf("< ---- >\n");
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

int main() {
    // global variable
    BTree* tree = initBTree(treeDegree);

    for (int i = 1; i <= 27; i++) {
        insertToBTree(tree, tree->root, i);
        printBTree(tree->root);
        printf("------------\n");
    }

    BTreeNode *child = getChild(tree->root, 10);

    // for (int i = 0; i < child->currentKeys; i++) {
        // printf("%d ", child->keys[i]);
    // }

    deleteFromBTree(tree, 19);
    deleteFromBTree(tree, 22);
    deleteFromBTree(tree, 18);
    // deleteFromBTree(tree, 22);

    printBTree(tree->root);
    //
    // deleteFromBTree(tree, 9);
    // // printBTree(tree->root);
    //
    // deleteFromBTree(tree, 7);
    // printBTree(tree->root);

    freeBTree(tree->root);
    free(tree);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree_internal.h"

void* safeMalloc(size_t size) {
    void* p = malloc(size);

    if (p == NULL) {
        perror("malloc");
        exit(1);
    }

    return p;
}

BTreeNode* initRoot(int maxKeys) {
    BTreeNode *node = (BTreeNode*)safeMalloc(sizeof(BTreeNode));

    node->keys = (int*)safeMalloc(sizeof(int) * maxKeys);
    node->children = (BTreeNode**)safeMalloc(sizeof(BTreeNode*) * (maxKeys + 1));
    node->parent = NULL;
    node->currentKeys = 0;
    node->isLeaf = 1;

    // make sure the children are set on NULL and keys on -1
    for (int i = 0; i < maxKeys; i++) {
        node->children[i] = NULL;
        node->keys[i] = 0;
    }
    node->children[maxKeys] = NULL;

    return node;
}

BTreeNode* initChild(BTreeNode* parent, int maxKeys) {
    BTreeNode *node = initRoot(maxKeys);

    node->parent = parent;
    node->isLeaf = 1;

    return node;
}

BTree* initBTree(int t) {
    BTree *tree = (BTree*)safeMalloc(sizeof(BTree));
    tree->t = t;
    tree->maxKeys = 2 * t - 1;
    tree->minKeys = t - 1;
    tree->root = initRoot(tree->maxKeys);

    return tree;
}

void printBTreeHelper(BTreeNode* root) {
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
        printBTreeHelper(root->children[i]);
    }
}

void printBTree(BTree* tree) {
    printBTreeHelper(tree->root);
}

void freeBTreeHelper(BTreeNode *root) {
    if (root == NULL) {
        return;
    }

    // traverse all children, root->curr + 1
    for (int i = 0; i <= root->currentKeys; i++) {
        if (root->children[i] != NULL) {
            freeBTreeHelper(root->children[i]);
        }
    }

    free(root->keys);
    free(root->children);
    free(root);
}

void freeBTree(BTree* tree) {
    freeBTreeHelper(tree->root);
    free(tree);
}

int getNewKeyPosition(int* array, int size, int key) {
    int i = 0;
    while (i < size && key > array[i]) {
        i++;
    }

    return i;
}

int hasKeyInNode(int* array, int size, int key) {
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

void insertSortedArray(int* array, int *size, int key) {
    if (*size == 0) {
        array[(*size)++] = key;
        return;
    }

    int pos = getNewKeyPosition(array, *size, key);

    // move the nodes 1 position to the right starting from pos
    for (int i = *size; i >= pos && i > 0; i--) {
        array[i] = array[i - 1];
    }

    array[pos] = key;
    (*size)++;
}

void deleteFromArray(int* array, int *size, int pos) {
    for (int i = pos; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }

    (*size)--;
}

void splitChild(BTreeNode* parent, int childIndex, int maxKeys) {
    BTreeNode *rightChild = initChild(parent, maxKeys);
    BTreeNode *child = parent->children[childIndex];
    int mid = child->currentKeys / 2;
    int promotedKey = child->keys[mid];
    int promotedKeyIndex = getNewKeyPosition(parent->keys, parent->currentKeys, promotedKey);

    // shift parent keys and children
    for (int i = parent->currentKeys + 1; i > promotedKeyIndex; i--) {
        parent->children[i] = parent->children[i - 1];

        if (i <= parent->currentKeys) {
            parent->keys[i] = parent->keys[i - 1];
        }
    }

    // update with the promoted key and right child
    parent->keys[promotedKeyIndex] = promotedKey;
    parent->children[promotedKeyIndex + 1] = rightChild;
    parent->currentKeys++;

    // copy data to the split node
    // math is screwed if your nodes are a max of 1 key for whatever reason
    rightChild->currentKeys = max((child->currentKeys - mid - 1), 1);

    memcpy(rightChild->keys, child->keys + mid + 1, rightChild->currentKeys * sizeof(int));
    memcpy(rightChild->children, child->children + mid + 1, (rightChild->currentKeys + 1) * sizeof(BTreeNode*));

    rightChild->isLeaf = child->isLeaf;
    child->currentKeys = mid;
}

SplitResult insertHelper(BTreeNode* root, int key, int maxKeys) {
    SplitResult result = {0, 0, NULL};

    // case 1: leaf
    if (root->isLeaf) {
        if (root->currentKeys < maxKeys) {
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
        BTreeNode *rightChild = initChild(root, maxKeys);
        result.rightChild = rightChild;

        // replace the current node with the left side
        int mid = tempSize / 2;
        root->currentKeys = 0;
        for (int i = 0; i < mid; i++) {
            root->keys[root->currentKeys++] = temp[i];
        }

        // set the right childs keys
        for (int i = mid + 1; i < tempSize; i++) {
            rightChild->keys[rightChild->currentKeys++] = temp[i];
        }

        // return the structure with the split data
        return result;
    }

    // case 2: it has propagated to an internal node
    // find the correct child top to bottom
    int pos = getNewKeyPosition(root->keys, root->currentKeys, key);
    BTreeNode *child = root->children[pos];

    SplitResult childSplit = insertHelper(child, key, maxKeys);

    // the node has been inserted without splits
    if (!childSplit.hasSplit) {
        return result;
    }

    // case 2A: child split, safe to add directly here
    if (root->currentKeys < maxKeys) {
        insertSortedArray(root->keys, &root->currentKeys, childSplit.promotedKey);

        // shift children to the correct place
        for (int i = root->currentKeys; i > pos + 1; i--) {
            root->children[i] = root->children[i - 1];
        }

        root->children[pos + 1] = childSplit.rightChild;
        childSplit.rightChild->parent = root;

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
    tempChildren[pos + 1] = childSplit.rightChild;

    int mid = tempKeyCount / 2;
    result.promotedKey = tempKeys[mid];

    result.hasSplit = 1;

    BTreeNode *rightChild = initChild(root->parent, maxKeys);
    result.rightChild = rightChild;

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
        rightChild->keys[rightChild->currentKeys] = tempKeys[i];
        rightChild->children[rightChild->currentKeys] = tempChildren[i];

        if (tempChildren[i])
            tempChildren[i]->parent = rightChild;

        rightChild->currentKeys++;
    }
    rightChild->children[rightChild->currentKeys] = tempChildren[tempKeyCount];
    if (tempChildren[tempKeyCount])
        tempChildren[tempKeyCount]->parent = rightChild;



    return result;
}

// first goes to leaf then propagates upward, possibly traversing the tree more than once, slightly faster with lower T
void insertToBTreeBottomToTop(BTree* tree, int key) {
    SplitResult result = insertHelper(tree->root, key, tree->maxKeys);

    if (result.hasSplit) {
        BTreeNode *newRoot = initRoot(tree->maxKeys);
        newRoot->keys[newRoot->currentKeys++] = result.promotedKey;
        newRoot->children[0] = tree->root;
        newRoot->children[1] = result.rightChild;

        result.rightChild->parent = newRoot;
        tree->root->parent = newRoot;

        tree->root->isLeaf = tree->root->children[0] == NULL;
        newRoot->isLeaf = 0;
        result.rightChild->isLeaf = result.rightChild->children[0] == NULL;

        tree->root = newRoot;
    }
}

// splits full children on the way down, guaranteeing a single tree traversal, slightly faster with higher T
void insertToBTreeTopToBottom(BTree* tree, int key) {
    // split root if necessary
    if (tree->root->currentKeys == tree->maxKeys) {
        BTreeNode *root = tree->root;
        BTreeNode *newRoot = initRoot(tree->maxKeys);
        BTreeNode *rightChild = initChild(newRoot, tree->maxKeys);
        int mid = root->currentKeys / 2;
        int promotedKey = root->keys[mid];

        // set left and right children of the new root
        newRoot->keys[newRoot->currentKeys++] = promotedKey;
        newRoot->children[0] = tree->root;
        newRoot->children[1] = rightChild;

        // old root becomes left child
        root->parent = newRoot;

        // set right child data
        rightChild->currentKeys = max(root->currentKeys - mid - 1, 1);
        root->currentKeys = mid;

        memcpy(rightChild->keys, root->keys + mid + 1, rightChild->currentKeys * sizeof(int));
        memcpy(rightChild->children, root->children + mid + 1, (rightChild->currentKeys + 1) * sizeof(BTreeNode*));

        // set the new root and fix leaves
        root->isLeaf = root->children[0] == NULL;
        rightChild->isLeaf = rightChild->children[0] == NULL;
        newRoot->isLeaf = 0;
        tree->root = newRoot;
    }

    // reach the correct child
    BTreeNode *child = tree->root;
    while (!child->isLeaf) {
        // check where to descend
        int childIndex = getNewKeyPosition(child->keys, child->currentKeys, key);

        // split the child in advance if it has full keys
        if (child->children[childIndex]->currentKeys == tree->maxKeys) {
            splitChild(child, childIndex, tree->maxKeys);
        }

        // recalc where to descend
        childIndex = getNewKeyPosition(child->keys, child->currentKeys, key);
        child = child->children[childIndex];
    }

    insertSortedArray(child->keys, &child->currentKeys, key);
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
    if (root->isLeaf) {
        return NULL;
    }

    // key is in a child
    // get right child position
    int pos = getNewKeyPosition(root->keys, root->currentKeys, key);
    return getChild(root->children[pos], key);
}

BTreeNode* getRightmostChild(BTreeNode* root) {
    if (root->isLeaf) {
        return root;
    }

    return getRightmostChild(root->children[root->currentKeys]);
}

void borrowFromLeft(BTreeNode* parent, int childIndex) {
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
    BTreeNode *child = parent->children[childIndex];
    BTreeNode *rightChild = parent->children[childIndex + 1];

    child->keys[child->currentKeys++] = parent->keys[childIndex];
    parent->keys[childIndex] = rightChild->keys[0];

    // shift keys to left
    for (int i = 0; i < rightChild->currentKeys - 1; i++) {
        rightChild->keys[i] = rightChild->keys[i + 1];
    }

    rightChild->currentKeys--;

    child->children[child->currentKeys] = rightChild->children[0];
    rightChild->children[0] = NULL;

    // shift children to left
    for (int i = 0; i <= rightChild->currentKeys; i++) {
        rightChild->children[i] = rightChild->children[i + 1];
    }
}

int mergeChildren(BTreeNode* parent, int childIndex, int minKeys) {
    // root case, treat by parent caller
    if (parent->parent == NULL && parent->currentKeys == 1) {
        return 1;
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
    if (parent->currentKeys < minKeys && parent->parent != NULL) {
        return fixUnderflow(parent->parent, getNewKeyPosition(parent->parent->keys, parent->parent->currentKeys, parent->keys[0]), minKeys);
    }

    return 0;
}

int fixUnderflow(BTreeNode* parent, int childIndex, int minKeys) {
    // first check if it can borrow from left
    if (childIndex > 0) {
        if (parent->children[childIndex - 1] != NULL && parent->children[childIndex - 1]->currentKeys > minKeys) {
            borrowFromLeft(parent, childIndex);
            return 0;
        }
    }

    // then from right
    if (childIndex < parent->currentKeys) {
        if (parent->children[childIndex + 1] != NULL && parent->children[childIndex + 1]->currentKeys > minKeys) {
            borrowFromRight(parent, childIndex);
            return 0;
        }
    }

    // forced to merge
    return mergeChildren(parent, childIndex, minKeys);
}

// only ever returns 1 if the root has to recursively merge in fixUnderflow -> mergeChildren
int deleteHelper(BTreeNode* root, int key, int minKeys) {
    // key doesnt exist in tree
    if (root == NULL) {
        return 0;
    }

    int pos = getNewKeyPosition(root->keys, root->currentKeys, key);

    // leaf delete operation
    if (root->isLeaf) {
        // if only the root is left
        if (root->parent == NULL) {
            deleteFromArray(root->keys, &root->currentKeys, pos);

            return 0;
        }

        int childIndex = getNewKeyPosition(root->parent->keys, root->parent->currentKeys, key);
        deleteFromArray(root->keys, &root->currentKeys, pos);

        // call helper function if the tree becomes invalid after delete
        if (root->currentKeys < minKeys) {
            return fixUnderflow(root->parent, childIndex, minKeys);
        }

        return 0;
    }

    // internal node delete operation
    BTreeNode *predecessor = getRightmostChild(root->children[pos]);
    int newKey = predecessor->keys[predecessor->currentKeys - 1];
    int childIndex = getNewKeyPosition(predecessor->parent->keys, predecessor->parent->currentKeys, newKey);

    // replace the separator in the parent with the max key in left subtree
    root->keys[pos] = newKey;

    // delete it from the leaf node
    deleteFromArray(predecessor->keys, &predecessor->currentKeys, predecessor->currentKeys - 1);

    if (predecessor->currentKeys < minKeys) {
        return fixUnderflow(predecessor->parent, childIndex, minKeys);
    }

    return 0;
}

// note that this doesnt work with duplicates
void deleteFromBTree(BTree* tree, int key) {
    BTreeNode *child = getChild(tree->root, key);

    int mergeRoot = deleteHelper(child, key, tree->minKeys);

    if (mergeRoot) {
        BTreeNode *leftChild = tree->root->children[0];
        BTreeNode *rightChild = tree->root->children[1];
        BTreeNode *root = tree->root;

        // append root node to left child
        leftChild->keys[leftChild->currentKeys++] = root->keys[0];

        // append the rest of the keys and children from the right child
        int base = leftChild->currentKeys;
        for (int i = 0; i < rightChild->currentKeys; i++) {
            leftChild->keys[leftChild->currentKeys] = rightChild->keys[i];
            leftChild->children[base + i] = rightChild->children[i];

            leftChild->currentKeys++;
        }
        leftChild->children[leftChild->currentKeys] = rightChild->children[rightChild->currentKeys];

        // kil the nodes
        tree->root = leftChild;
        leftChild->parent = NULL;

        free(root->keys);
        free(root->children);
        free(root);

        free(rightChild->keys);
        free(rightChild->children);
        free(rightChild);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "btree.h"


int main() {
    BTree* tree = initBTree(3);

    // FILE *input = fopen("../b-tree/keys.txt", "r");
    // char buffer[100];
    // int i = 0;
    //
    // while (fgets(buffer, 100, input)) {
    //     int num = atoi(buffer);
    //     insertToBTreeTopToBottom(tree, num);
    //
    //     if (i % 100000 == 0) {
    //         printf("Iter %d\n", i);
    //     }
    //
    //     i++;
    // }

    for (int i = 1; i <= 30; i++) {
        insertToBTreeBottomToTop(tree, i);
    }

    deleteFromBTree(tree, 12);

    printBTree(tree);
    freeBTree(tree);

    return 0;
}
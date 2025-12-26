#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gperftools/profiler.h>
#include "btree.h"

int main() {
    // ProfilerStart("cpu.prof");

    BTree* tree = initBTree(3);

    // FILE *input = fopen("../keys.txt", "r");
    // char buffer[100];
    // int i = 0;
    //
    // while (fgets(buffer, 100, input)) {
    //     int num = atoi(buffer);
    //     insertToBTreeTopToBottom(tree, num);
    //
    //     // if (i % 100000 == 0) {
    //     //     printf("Iter %d\n", i);
    //     // }
    //     //
    //     // i++;
    // }
    //
    // fclose(input);

    for (int i = 1; i <= 30; i++) {
        insertToBTreeTopToBottom(tree, i);
    }

    // deleteFromBTree(tree, 16);
    // deleteFromBTree(tree, 15);
    // deleteFromBTree(tree, 14);
    // deleteFromBTree(tree, 15);

    printBTree(tree);
    freeBTree(tree);

    // ProfilerStop();

    return 0;
}
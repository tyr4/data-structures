#include <stdio.h>
#include <stdlib.h>

#include "int_multimap.h"
#include "generic_sll.h"

int main() {
    Multimap multimap;
    multimap.mapSize = 10;
    multimap.sllSize = 5;
    multimap.map = createMultimap(multimap.mapSize);

    for (int i = 0; i < 45; i++) {
        addToMultimap(&multimap, i);
    }

    printMultimap(&multimap);

    for (int i = 45; i < 101; i++) {
        addToMultimap(&multimap, i);
    }

    printMultimap(&multimap);
}


#include "memtable.h"

#include <iostream>
#include <assert.h> /* assert */

int main()
{
    Memtable mt = Memtable(108);
    mt.put(5, 1);
    mt.put(6, 3);
    mt.put(2, 8);
    mt.put(1, 9);
    mt.put(1, 7);
    mt.put(9, 8);
    mt.put(3, 0);
    mt.put(10, 1);
    mt.put(11, 17);

    mt.print();

    int actual = 17;
    assert(actual == mt.get(11));
}


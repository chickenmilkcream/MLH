// open(name)
// close()
// get(key)
// should search memtable, then all SSTs in order of youngest to oldest
// BST search over memtable
// binary search over SSTs

// put(key, value)
// scan(key1, key2)     

// memtable (memtable_size) balanced binary tree in memory --> serialized to SST in storage


// class KeyValueStore


// class Memtable
// 4KB page size

#include <iostream>
#include "kv_store.h"
#include "memtable.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello World" << std::endl;
    

    KeyValueStore kv_store = KeyValueStore(69420);


    kv_store.put(69, 1);
    
    // return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}

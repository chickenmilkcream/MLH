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

    kv_store.put((db_key_t)69, (db_val_t)1);

    // return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value

        
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

    std::cout << mt.get(5) << std::endl;
    std::cout << mt.get(6) << std::endl;
    std::cout << mt.get(2) << std::endl;
    std::cout << mt.get(1) << std::endl;
    std::cout << mt.get(1) << std::endl;
    std::cout << mt.get(9) << std::endl;
    std::cout << mt.get(3) << std::endl;
    std::cout << mt.get(10) << std::endl;
    std::cout << mt.get(11) << std::endl;

    vector<pair<db_key_t, db_val_t> > mt_vector = mt.scan(4, 10);
    for (pair<db_key_t, db_val_t> pair : mt_vector)
    {
        cout << pair.first << ", " << pair.second << endl;
    }
    cout << "size: " << mt_vector.size() << endl;

    // std::ofstream outputFile("sst_1.bin", std::ios::binary);
    // for (const auto &p : mt_vector)
    // {
    //   outputFile.write((char *)&p, sizeof(p));
    // }
    // outputFile.close();
}

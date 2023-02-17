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
    Memtable mt = Memtable(10);
    assert(mt.max_size == 10);

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

    // Testing the get functionality
    assert(mt.get(5) == 1);
    assert(mt.get(6) == 3);
    assert(mt.get(2) == 8);
    assert(mt.get(1) == 7);
    assert(mt.get(9) == 8);
    assert(mt.get(3) == 0);
    assert(mt.get(10) == 1);
    assert(mt.get(11) == 17);
    try { // making sure that invalid_argument is thrown when trying to retrieve non existent key
        mt.get(-100);
        assert(false);
    } catch(invalid_argument e) {
    }

    // Testing the scan functionality

    // Testing normal scan
    vector<pair<db_key_t, db_val_t> > mt_vector = mt.scan(4, 10);
    vector<pair<db_key_t, db_val_t> > expected_vector;
    expected_vector.push_back(pair<db_key_t, db_val_t>(5, 1));
    expected_vector.push_back(pair<db_key_t, db_val_t>(6, 3));
    expected_vector.push_back(pair<db_key_t, db_val_t>(9, 8));
    expected_vector.push_back(pair<db_key_t, db_val_t>(10, 1));

    assert(mt_vector.size() == 4);
    assert(mt_vector == expected_vector);
    expected_vector.clear();
    mt_vector.clear();

    // Testing scan of 1 element
    mt_vector = mt.scan(10, 10);
    expected_vector.push_back(pair<db_key_t, db_val_t>(10, 1));

    assert(mt_vector.size() == 1);
    assert(mt_vector == expected_vector);
    expected_vector.clear();
    mt_vector.clear();

    // Testing scan of 0 elements
    mt_vector = mt.scan(11, 10);

    assert(mt_vector.size() == 0);
    assert(mt_vector == expected_vector);

    // Testing the serialize functionality
    // todo: jason you got this I believe in you

    // std::ofstream outputFile("sst_1.bin", std::ios::binary);
    // for (const auto &p : mt_vector)
    // {
    //   outputFile.write((char *)&p, sizeof(p));
    // }
    // outputFile.close();
}

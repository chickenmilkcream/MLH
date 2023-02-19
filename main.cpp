#include <iostream>
#include <cassert>
#include "kv_store.h"
#include "memtable.h"

int main(int argc, char *argv[])
{
    // Want the max size to be 10 key-value pairs with each pair taking up 16 bytes
    KeyValueStore kv = KeyValueStore(160); 

    kv.put(5, 1);
    kv.put(6, 3);
    kv.put(2, 8);
    kv.put(1, 9);
    kv.put(1, 7);
    kv.put(9, 8);
    kv.put(3, 0);
    kv.put(10, 1);
    kv.put(11, 17);
    kv.put(69, 17);

    kv.print();

    /*
    Testing the get functionality
    */
    
    assert(kv.get(5) == 1);
    assert(kv.get(6) == 3);
    assert(kv.get(2) == 8);
    assert(kv.get(1) == 7);
    assert(kv.get(9) == 8);
    assert(kv.get(3) == 0);
    assert(kv.get(10) == 1);
    assert(kv.get(11) == 17);
    assert(kv.get(69) == 17);

    try { // making sure that invalid_argument is thrown when trying to retrieve non existent key
        kv.get(100);
        assert(false);
    } catch(invalid_argument e) {
    }

    /*
    Testing the scan functionality
    */

    // Testing normal scan
    vector<pair<db_key_t, db_val_t> > mt_vector = kv.scan(4, 10);
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
    mt_vector = kv.scan(10, 10);
    expected_vector.push_back(pair<db_key_t, db_val_t>(10, 1));

    assert(mt_vector.size() == 1);
    assert(mt_vector == expected_vector);
    expected_vector.clear();
    mt_vector.clear();

    // Testing scan of 0 elements
    mt_vector = kv.scan(11, 10);

    assert(mt_vector.size() == 0);
    assert(mt_vector == expected_vector);

    // Testing scan of all elements
    mt_vector = kv.scan(0, 99999999);
    assert(mt_vector.size() == 9);

    /*
    Testing the serialize functionality
    */

    // Testing sst_1.bin has correct elements
    kv.put(699, 420);

    kv.read_from_file("sst_1.bin"); // Should expect to see everything up until this point

    // Testing sst_2.bin has correct elements
    kv.put(5, 1);
    kv.put(6, 3);
    kv.put(2, 8);
    kv.put(1, 9);
    kv.put(8, 7);
    kv.put(9, 8);
    kv.put(3, 0);
    kv.put(10, 1);
    kv.put(11, 17);
    kv.put(69, 17); // All of this will be added to sst_2.bin
    kv.put(99, 17);
    kv.print(); // Should expect to only expect to see 99 in the new memtable

    kv.read_from_file("sst_2.bin");

    // TODO BUG: if we try to put 3 consequtive elements like this that requires rotation, it seg faults.
    kv = KeyValueStore(2000);
    kv.put(69, 17);
    kv.put(68, 17);
    kv.put(67, 17);
    kv.print();
}

#include <iostream>
#include <cassert>
#include "kv_store.h"
#include "memtable.h"

int main(int argc, char *argv[])
{
    int memtable_size = 144;
    string eviction_policy = "clock";
    int initial_num_bits = 2;
    int maximum_num_bits = 4;
    int maximum_num_pages = 10;

    // Want the max size to be 9 key-value pairs with each pair taking up 16 bytes

    std::cout << "Running test cases for Phase 1 \n";
    std::cout << "Creating a database store with a max size of 10 key value pairs \n";
    std::cout << "--------------------------------------------------------------- \n";

    KeyValueStore kv = KeyValueStore(memtable_size, eviction_policy, initial_num_bits, maximum_num_bits, maximum_num_pages);

    // Want each page size to be 3 key-value pairs so that each SST will have 5 pages
    kv.set_page_size(48);

    kv.put(5, 1);
    kv.put(6, 3);
    kv.put(2, 8);
    kv.put(1, 9);
    kv.put(1, 7);
    kv.put(9, 8);
    kv.put(3, 0);
    kv.put(10, 1);
    kv.put(11, 17);
    
    std::cout << "* Insert 8 pairs passed \n";

    assert(kv.get(5) == 1);
    assert(kv.get(6) == 3);
    assert(kv.get(2) == 8);
    assert(kv.get(1) == 7);
    assert(kv.get(9) == 8);
    assert(kv.get(3) == 0);
    assert(kv.get(10) == 1);
    assert(kv.get(11) == 17);

    std::cout << "* Get key passed \n";

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

    std::cout << "* Scan range (4, 10) on memtable passed \n";

    // Testing scan of 1 element
    mt_vector = kv.scan(10, 10);
    expected_vector.push_back(pair<db_key_t, db_val_t>(10, 1));

    assert(mt_vector.size() == 1);
    assert(mt_vector == expected_vector);
    expected_vector.clear();
    mt_vector.clear();

    std::cout << "* Scan range (10, 10) on memtable passed \n";

    // Testing scan of 0 elements
    mt_vector = kv.scan(11, 10);

    assert(mt_vector.size() == 0);
    assert(mt_vector == expected_vector);

    std::cout << "* Scan range (11, 10) on memtable passed \n";

    // Testing scan of all elements
    mt_vector = kv.scan(0, 99999999);
    assert(mt_vector.size() == 8);

    std::cout << "* Scan range (0, 99999999) on memtable passed \n";

    // Testing sst_1.bin has correct elements
    kv.put(911, 420); // Add one more element so the memtable would be full

    std::cout << "* Serialize first memtable passed \n";
    std::cout << "Results of sst_1.bin: \n";

    kv.read_from_file("sst_1.bin");

    // Testing sst_2.bin has correct elements
    kv.put(95, 1);
    kv.put(96, 3);
    kv.put(92, 8);
    kv.put(91, 9);
    kv.put(98, 7);
    kv.put(99, 8);
    kv.put(93, 0);
    kv.put(910, 1);
    kv.put(911, 17);

    std::cout << "* Serialize second memtable passed \n";
    std::cout << "Results of sst_2.bin: \n";

    kv.read_from_file("sst_2.bin");

    // Testing scan of all elements
    kv.put(70, 17);
    mt_vector = kv.scan(69, 98);
    assert(mt_vector.size() == 7);

    std::cout << "* Scan range (69, 98) on memtable passed \n";
    std::cout << "Results from memtable, sst_2.bin, sst_1.bin: \n";

    for (auto p : mt_vector)
    {
        cout << p.first << " " << p.second << endl;
    }

    // Testing get of an element in SST
    std::cout << "Results of sst_1.bin: \n";
    kv.read_from_file("sst_1.bin");

    std::cout << "Results of sst_2.bin: \n";
    kv.read_from_file("sst_2.bin");

    assert(kv.get(70) == 17); // From memtable
    assert(kv.get(93) == 0);  // From sst_2.bin on page 1
    assert(kv.get(91) == 9);  // From sst_2.bin on page 1
    assert(kv.get(96) == 3);  // From sst_2.bin on page 2
    assert(kv.get(911) == 17);  // From sst_2.bin on page 3
    assert(kv.get(1) == 7);      // From sst_1.bin on page 1
    assert(kv.get(5) == 1);      // From sst_1.bin on page 2
    assert(kv.get(11) == 17);   // From sst_1.bin on page 3

    try
    { // making sure that invalid_argument is thrown when trying to retrieve non existent key
        kv.get(100);
        assert(false);
    }
    catch (invalid_argument e)
    {
    }
    std::cout << "* Get key in memtable, sst_2.bin, sst_1.bin passed \n";

    // // Testing closing db
    // // Note that an element (70, 17) was added to the memtable so that it will get written to a new SST when db closes
    // kv.close_db();
    // std::cout << "* Close DB passed \n";
    // std::cout << "Results of sst_3.bin: \n";

    // kv.read_from_file("sst_3.bin");
}
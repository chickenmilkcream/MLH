#include <iostream>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include "kv_store.h"
#include <unordered_map>
#include <vector>
#include <fcntl.h>

using namespace std;

int main(int argc, char *argv[])
{
    string eviction_policy = "LRU";
    int initial_num_bits = 2;
    int maximum_bp_size = 100000;
    int maximum_num_items_threshold = 10;  

    /* ==================== B-TREE GET, PUT, SCAN TESTS ==================== */
    cout << "============== B-TREE GET, PUT, SCAN TESTS ==============" << endl;

    /* 
     * each page stores 4096 / 16 = 256 key-value pairs (for terminal nodes) or
     * 4096 / 8 = 512 keys (for non-terminal nodes)
     */
    // size_t n = 256 + 1; // height 2
    size_t n = 256 * (256 + 1); // height 2
    // size_t n = 256 * (256 + 1) + 1; // height 3
    // size_t n = 256 * (256 * (256 + 1) + 1) + 1; // height 4

    // KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE + 1, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes zero SSTs (all in memory)
    // KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes one SST
    // KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE / 2, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)
    
    // KeyValueStore db = KeyValueStore(4096 * DB_PAIR_SIZE, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes n / 4096 SSTs


    // for (int i = 0; i < n; i++) {
    //     db.put(i, i);
    // }

    // db.read_from_file("sst.5.1.bin");

    // for (int i = 0; i < n; i++) {
    //     db.get(i);
    //     if (i > 0 && i % 1000 == 0) {
    //         cout << "Tested " << i << " get operations so far..." << endl;
    //     }
    // }

    // db.close_db();

    // KeyValueStore db = KeyValueStore(3 * DB_PAIR_SIZE, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes n / 4096 SSTs
    // sst.1.1.bin should be created
    // db.put(0, 1);
    // db.put(1, 1);
    // db.put(2, 1);
    // db.read_from_file("sst.1.1.bin");
    // // sst.1.2.bin should be created
    // db.put(0, 100);
    // db.put(1, 3);
    // db.del(2);
    // db.read_from_file("sst.2.1.bin");

    // db.put(3, 1);
    // db.put(4, 1);
    // db.put(5, 1);
    // db.read_from_file("sst.1.1.bin");

    // db.put(3, 1);
    // db.put(4, 1);
    // db.del(1);
    // db.read_from_file("sst.3.1.bin");
    // // sst.2.1.bin should be created from merging
    // assert(db.get(0) == 100);
    // try
    // {
    //     db.get(2);
    //     assert(false);
    // }
    // catch (invalid_argument e)
    // {
    // }

    // db.delete_sst_files();

    cout << "SCAN tests" << endl;
    KeyValueStore db = KeyValueStore(2 * DB_PAIR_SIZE, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes n / 4096 SSTs
    db.put(0, 1);
//    db.get(0);
    db.put(1, 1);
    db.del(0);
    db.del(1);
    sleep(1); // sleep for 1 second
    db.read_from_file("sst.2.1.bin");

    assert(db.scan(-1, 2).size() == 0);
    db.put(0, 5);
    db.put(1, 1);
    db.read_from_file("sst.1.1.bin");
    db.put(2, 1);
    db.del(1);
    db.read_from_file("sst.3.1.bin");
    db.put(3, 1);
    assert(db.scan(-1, 4).size() == 3); // 0, 2, 3
    db.del(3);
    assert(db.scan(-1, 4).size() == 2); // 0, 2
    db.del(2);
    assert(db.scan(-1, 4).size() == 1); // 0

    cout << "\n\n\nk everything passes now don't touch the code or we might break something uwu\n\n\n\n" << endl;
}



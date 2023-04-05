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
    int maximum_bp_size = 1000;
    int maximum_num_items_threshold = 10;  

    /* ==================== B-TREE GET, PUT, SCAN TESTS ==================== */
    cout << "============== B-TREE GET, PUT, SCAN TESTS ==============" << endl;

    /* 
     * each page stores 4096 / 16 = 256 key-value pairs (for terminal nodes) or
     * 4096 / 8 = 512 keys (for non-terminal nodes)
     */
    // size_t n = 256 + 1; // height 2
    size_t n = 256 * (256 + 1) + 1; // height 2
    // size_t n = 256 * (256 + 1) + 1; // height 3
    // size_t n = 256 * (256 * (256 + 1) + 1) + 1; // height 4

    // KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE + 1, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes zero SSTs (all in memory)
    // KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes one SST
    KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE / 2, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)

   

    for (int i = 0; i < n; i++) {
        db.put(i, i);
    }


    db.read_from_file("sst_1.bin");
    db.read_from_file("sst_2.bin");

    db.compact_files({"sst_1.bin", "sst_2.bin"});
    db.read_from_file("sst_0_0");

    db.close_db();
    /* ==================== B-TREE GET, PUT, SCAN TESTS ==================== */
}



#include <iostream>
#include <cassert>
#include "kv_store.h"
#include "memtable.h"

#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    int n = 256 + 1; // height 2
    // int n = 256 * (256 + 1) + 1; // height 3
    // int n = 256 * (256 * (256 + 1) + 1) + 1; // height 4
    KeyValueStore db = KeyValueStore(n * 16); // this writes an SST 
    // KeyValueStore db = KeyValueStore(n * 16 + 1); // this does NOT write an SST

    // each page stores 4096 / 16 = 256 key-value pairs (for terminal nodes) or 
    // 4096 / 8 = 512 keys (for non-terminal nodes)

    for (int i = 0; i < n; i++) {
        db.put(2 * i, i);
    }

    // db.read_from_file("sst_1.bin"); 
    // cout << db.get(1, search_alg::b_tree_search) << endl;
    cout << db.get(0, search_alg::binary_search) << endl;
}
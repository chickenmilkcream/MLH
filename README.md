# CSC443 Final Project

### Verification
Here are some make commands that could be potentially useful.  

- `make all_tests`
- `make phase1_experiments`
- `make phase2_experiment1`
- `make phase2_experiment2`
- `make clean`

### Notes
For each of the items in the list of design elements/experiments, there's a corresponding comment in the code files that point to where it's being implemented. For example, search for "`KV-store get API (1) - 1 points`".

### Usage
All the database functionalities are listed in `kv_store.h`. To test it out, initialize an instance like this:

```
string eviction_policy = "LRU";          // This is the eviction policy, should be "LRU" or "clock"
int initial_num_bits = 2;                // This is the initial number of prefix bits in the directory
int maximum_bp_size = 100000;            // This is the max number of bytes that the buffer pool can contain before eviction policy should kick in
int maximum_num_items_threshold = 10;    // This is the maximum number of items we can have in the buffer pool before expansion, which results in doubling
int memtable_size = 4096 * DB_PAIR_SIZE; // This is the maximum size of the memtable before serialization

KeyValueStore db = KeyValueStore(memtable_size, eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold);
```

Here are some examples of commands you can run to interact with the database:
For a full list, please check `kv_store.h`.
```
db_key_t key = 1;
db_val_t val = 1;
db_key_t min_key = -9;
db_key_t max_key = 9;
search_alg binary_search = search_alg::binary_search;
search_alg btree_search = search_alg::b_tree_search
db.put(key, val);
db_val_t result_get = db.get(key, binary_search);
vector<pair<db_key_t, db_val_t>> result_scan = scan(min_key, max_key, b_tree_search);
db.del(key);
db.print();
db.close_db();
```

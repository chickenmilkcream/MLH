#ifndef KV_STORE_H
#define KV_STORE_H

#include <cstdint>
#include <string>

#include "memtable.h"
#include "bp_directory.h"

using namespace std;

class KeyValueStore
{
public:
    KeyValueStore(int memtable_size = 0, string eviction_policy = "clock", int initial_num_bits = 0, int maximum_bp_size = 0, int maximum_num_items_threshold = 0);
    void open_db(string db);
    void close_db();
    db_val_t get(db_key_t key);
    void put(db_key_t key, db_val_t val);
    vector<pair<db_key_t, db_val_t> > scan(db_key_t min_key, db_key_t max_key);
    void read_from_file(const char *filename);
    void print();
    void set_get_method(string get_method);
    void set_page_size(int page_size);
    BPDirectory buffer_pool;

private:
    Memtable memtable;

    int sst_num;
    int memtable_size;
    int page_size;
    string get_method;

    void serialize();
    void write_to_file(vector<pair<db_key_t, db_val_t> > vector_mt);
    int binary_search_smallest(int file, db_key_t key);
    int binary_search_page_containing_target(string sst_name, int file, db_key_t target);
    int binary_search_target_within_page(int file, int page, db_key_t key);
};

#endif
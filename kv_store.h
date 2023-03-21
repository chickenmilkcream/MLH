#ifndef KV_STORE_H
#define KV_STORE_H

#include <cstdint>
#include <string>

#include "memtable.h"

#define PAGE_SIZE 4096 // must be 4096 for direct I/O

using namespace std;

enum search_alg { binary_search, b_tree_search };

class KeyValueStore
{
public:
  KeyValueStore(size_t memtable_size = 0);
  void open_db(string db);
  void close_db();
  db_val_t get(db_key_t key, search_alg alg = b_tree_search);
  void put(db_key_t key, db_val_t val);
  vector<pair<db_key_t, db_val_t> > scan(db_key_t min_key, db_key_t max_key);
  void read_from_file(const char *filename);
  void print();

private:
  Memtable memtable;
  int num_sst;
  size_t memtable_size;
  void serialize();
  void write_to_file(const char *filename,
                     vector<size_t> sizes,
                     vector<vector<db_key_t> > non_terminal_nodes,
                     vector<pair<db_key_t, db_val_t> > terminal_nodes);
  int binary_search_smallest(int file, db_key_t key);
  int binary_search_exact(int file, db_key_t key);
};

#endif

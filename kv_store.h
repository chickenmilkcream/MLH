#ifndef KV_STORE_H
#define KV_STORE_H

#include <cstdint>
#include <string>

#include "memtable.h"
#include "bp_directory.h"

#define PAGE_SIZE 4096 // must be 4096 for direct I/O

using namespace std;

enum search_alg { binary_search, b_tree_search };

class KeyValueStore
{
public:
  BPDirectory buffer_pool;

  KeyValueStore(size_t memtable_size = 0, string eviction_policy = "clock", int initial_num_bits = 0, int maximum_bp_size = 0, int maximum_num_items_threshold = 0);
  void open_db(string db);
  void close_db();
  db_val_t get(db_key_t key, search_alg alg = search_alg::b_tree_search);
  void put(db_key_t key, db_val_t val);
  vector<pair<db_key_t, db_val_t> > scan(db_key_t min_key,
                                         db_key_t max_key,
                                         search_alg alg = search_alg::b_tree_search);
  void read_from_file(const char *filename);
  void print();

private:
  Memtable memtable;
  size_t sst_count;
  size_t memtable_size;
  
  void serialize();
  void bpread(string filename, int fd, void *buf, off_t fp);
  void write_to_file(const char *filename,
                     vector<size_t> sizes,
                     vector<vector<db_key_t> > non_terminal_nodes,
                     vector<pair<db_key_t, db_val_t> > terminal_nodes);
  void sizes(string filename, int fd, off_t &fp, vector<size_t> &sizes, size_t &height);
  void binary_search(string filename,
                     int fd,
                     db_key_t key,
                     vector<size_t> sizes,
                     size_t height,
                     off_t &start,
                     off_t &fp);
  void b_tree_search(string filename,
                     int fd,
                     db_key_t key,
                     vector<size_t> sizes,
                     size_t height,
                     off_t &start,
                     off_t &fp);
};

#endif

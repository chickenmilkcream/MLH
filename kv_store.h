#ifndef KV_STORE_H
#define KV_STORE_H

#include <cstdint>
#include <string>

#include "memtable.h"

using namespace std;

class KeyValueStore {
 public:
  KeyValueStore(unsigned int memtable_size) : memtable(memtable_size){};
  void open(string db);
  void close();
  db_val_t get(db_key_t key);
  void put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t>> &pairs);

 private:
  Memtable memtable;
  int num_sst;
  void serialize();
};

#endif

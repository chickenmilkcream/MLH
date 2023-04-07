#ifndef MEMTABLE_H
#define MEMTABLE_H

#include "avl_tree.h"

using namespace std;

class Memtable
{
public:
  size_t max_size; // Default should be 4KB
  size_t size;     // The number of bytes in the memtable

  Memtable(size_t memtable_size = 0);
  db_val_t get(db_key_t key);
  void put(db_key_t key, db_val_t val);
  vector<pair<db_key_t, db_val_t> > scan(db_key_t min_key, db_key_t max_key);
  vector<pair<db_key_t, db_val_t> > scan(db_key_t min_key, db_key_t max_key, set<db_key_t> *deleted_keys);
  // void del(db_key_t key);
  void print();

private:
  AVLTree tree;
};

#endif
#include <cmath>
#include <iostream>

#include "memtable.h"

using namespace std;

Memtable::Memtable(uint64_t memtable_size)
{
  this->max_size = memtable_size;
  this->size = 0;
  this->tree = AVLTree();
}

db_val_t Memtable::get(db_key_t key) { return this->tree.get(key); }

void Memtable::put(db_key_t key, db_val_t val)
{
  if (!this->tree.has(key))
  {
    this->size += sizeof(db_key_t) + sizeof(db_val_t); // 16 bytes per key-value pair
  }
  this->tree.put(key, val);
}

vector<pair<db_key_t, db_val_t> > Memtable::scan(db_key_t min_key, db_key_t max_key)
{
  vector<pair<db_key_t, db_val_t> > pairs;
  this->tree.scan(min_key, max_key, pairs);
  return pairs;
}

void Memtable::print() { this->tree.print(); }

#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

typedef uint64_t db_key_t;
typedef uint64_t db_val_t;

class AVLNode {
 public:
  db_key_t key;
  db_val_t val;
  AVLNode *left;
  AVLNode *right;
  unsigned int height;

  AVLNode(db_key_t key, db_val_t val);
  AVLNode *rotate_left();
  AVLNode *rotate_right();
  AVLNode *put(db_key_t key, db_val_t val);
  db_val_t get(db_key_t key);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t>> &pairs);
  void print(string prefix, bool is_left);
};

class AVLTree {
 public:
  AVLNode *root;

  AVLTree();
  void put(db_key_t key, db_val_t val);
  db_val_t get(db_key_t key);
  vector<pair<db_key_t, db_val_t>> scan(db_key_t min_key, db_key_t max_key);
  void print();
};

class Memtable {
 public:
  unsigned int max_size;
  unsigned int size;
  AVLTree tree;

  Memtable(unsigned int memtable_size);
  void put(db_key_t key, db_val_t val);
  db_val_t get(db_key_t key);
  vector<pair<db_key_t, db_val_t>> scan(db_key_t min_key, db_key_t max_key);
  void print();
};

#endif
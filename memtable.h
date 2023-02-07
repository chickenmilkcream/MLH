#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace std;

typedef uint64_t db_key_t;
typedef uint64_t db_val_t;

class AVLNode
{
public:
  db_key_t key;
  db_val_t val;

  AVLNode(db_key_t key, db_val_t val);
  bool has(db_key_t key);
  db_val_t get(db_key_t key);
  AVLNode *put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs);
  void print(string prefix, bool is_left);

private:
  AVLNode *left;
  AVLNode *right;
  unsigned int height;

  AVLNode *rotate_left();
  AVLNode *rotate_right();
};

class AVLTree
{
public:
  AVLTree();
  bool has(db_key_t key);
  db_val_t get(db_key_t key);
  void put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs);
  void print();

private:
  unique_ptr<AVLNode> root;
};

class Memtable
{
public:
  unsigned int max_size;
  unsigned int size;

  Memtable(unsigned int memtable_size);
  db_val_t get(db_key_t key);
  void put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs);
  void print();

private:
  AVLTree tree;
};

#endif
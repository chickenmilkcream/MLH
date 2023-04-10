#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "avl_node.h"

using namespace std;

class AVLTree
{
public:
  AVLTree();
  bool has(db_key_t key);
  db_val_t get(db_key_t key);
  void put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs, set<db_key_t> *deleted_keys);
  void print();

private:
  shared_ptr<AVLNode> root;
};

#endif

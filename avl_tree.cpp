#include <iostream>

#include "avl_tree.h"

using namespace std;

AVLTree::AVLTree() {
}

bool AVLTree::has(db_key_t key)
{
  if (this->root)
  {
    return this->root->has(key);
  }
  return false;
}

db_val_t AVLTree::get(db_key_t key)
{
  if (this->root)
  {
    if (this->root->key == key)
    {
      return this->root->val;
    }
    return this->root->get(key);
  }
  throw invalid_argument("Key not found");
}

void AVLTree::put(db_key_t key, db_val_t val)
{
  if (this->root)
  {
    this->root = this->root->put(key, val);
  }
  else
  {
    this->root = make_shared<AVLNode>(key, val);
  }
}

void AVLTree::scan(db_key_t min_key, db_key_t max_key,
                   vector<pair<db_key_t, db_val_t> > &pairs)
{
  if (this->root)
  {
    this->root->scan(min_key, max_key, pairs);
  }
}

void AVLTree::del(db_key_t key)
{
  if (this->root)
  {
    this->root = this->root->del(key);
  }
  else
  {
    throw invalid_argument("Key not found");
  }
}

void AVLTree::print()
{
  if (this->root)
  {
    this->root->print("", false);
  }
  else
  {
    cout << endl;
  }
}

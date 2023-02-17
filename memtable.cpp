// #include "avlnode.h"
#include "memtable.h"

#include <cmath>
#include <iostream>

using namespace std;

AVLNode::AVLNode(db_key_t key, db_val_t val)
{
  this->key = key;
  this->val = val;
  this->height = 1;
}

shared_ptr<AVLNode> AVLNode::rotate_left()
{
  shared_ptr<AVLNode> right = this->right;
  this->right = right->left;
  right->left = shared_from_this();
  this->height = max(this->left ? this->left->height : 0,
                     this->right ? this->right->height : 0) +
                 1;
  right->height =
      max(this->height, right->right ? right->right->height : 0) + 1;
  return right;
}

shared_ptr<AVLNode> AVLNode::rotate_right()
{
  shared_ptr<AVLNode> left = this->left;
  this->left = left->right;
  left->right = shared_from_this();
  this->height = max(this->left ? this->left->height : 0,
                     this->right ? this->right->height : 0) +
                 1;
  right->height = max(left->left ? left->left->height : 0, this->height) + 1;
  return left;
}

bool AVLNode::has(db_key_t key)
{
  if (this->key > key)
  {
    if (this->left)
    {
      return this->left->has(key);
    }
    return false;
  }
  if (this->key < key)
  {
    if (this->right)
    {
      return this->right->has(key);
    }
    return false;
  }
  return true;
}

db_val_t AVLNode::get(db_key_t key)
{
  if (this->key > key)
  {
    if (this->left)
    {
      return this->left->get(key);
    }
    throw std::invalid_argument("Key not found");
    return 69420;
  }
  if (this->key < key)
  {
    if (this->right)
    {
      return this->right->get(key);
    }
    throw std::invalid_argument("Key not found");
    return 69420;
  }
  return this->val;
}

shared_ptr<AVLNode> AVLNode::put(db_key_t key, db_val_t val)
{
  if (this->key > key)
  {
    if (!this->left)
    {
      this->left = make_shared<AVLNode>(key, val);
    }
    else
    {
      this->left = this->left->put(key, val);
    }
  }
  else if (this->key < key)
  {
    if (!this->right)
    {
      this->right = make_shared<AVLNode>(key, val);
    }
    else
    {
      this->right = this->right->put(key, val);
    }
  }
  else
  {
    this->val = val; // already exists
    return shared_from_this();
  }

  this->height = max(this->left ? this->left->height : 0,
                     this->right ? this->right->height : 0) +
                 1;

  int balance = (this->left ? this->left->height : 0) -
                (this->right ? this->right->height : 0);

  if (balance < -1 && this->right->key < key)
  {
    return this->rotate_left();
  }
  if (balance < -1 && this->right->key > key)
  {
    this->right = this->right->rotate_right();
    return this->rotate_left();
  }
  if (balance > 1 && this->left->key > key)
  {
    return this->rotate_right();
  }
  if (balance > 1 && this->left->key < key)
  {
    this->left = this->left->rotate_left();
    return this->rotate_right();
  }

  return shared_from_this();
}

void AVLNode::scan(db_key_t min_key, db_key_t max_key,
                   vector<pair<db_key_t, db_val_t> > &pairs)
{
  if (this->key > min_key)
  {
    if (this->left)
    {
      this->left->scan(min_key, max_key, pairs);
    }
  }
  if (this->key >= min_key && this->key <= max_key)
  {
    pair<db_key_t, db_val_t> pair;
    pair = make_pair(this->key, this->val);
    pairs.push_back(pair);
  }
  if (this->key < max_key)
  {
    if (this->right)
    {
      this->right->scan(min_key, max_key, pairs);
    }
  }
}

// modified from
// https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
void AVLNode::print(string prefix, bool is_left)
{
  cout << prefix + (is_left ? "├──" : "└──") << "(" << this->key << ":"
       << this->val << ")" << endl;

  if (this->left)
  {
    this->left->print(prefix + (is_left ? "│   " : "    "), true);
  }
  else
  {
    cout << prefix + (is_left ? "│   ├──NULL" : "    ├──NULL") << endl;
  }
  if (this->right)
  {
    this->right->print(prefix + (is_left ? "│   " : "    "), false);
  }
  else
  {
    cout << prefix + (is_left ? "│   └──NULL" : "    └──NULL") << endl;
  }
}

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
  return -1;
}

void AVLTree::put(db_key_t key, db_val_t val)
{
  if (!this->root)
  {
    this->root = make_shared<AVLNode>(key, val);
  }
  else
  {
    this->root->put(key, val);
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
    this->size += 16; // 16 bytes per key-value pair
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

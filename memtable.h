#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <cstdint>
#include <vector>

using namespace std;

typedef uint64_t db_key_t;
typedef uint64_t db_val_t;
typedef uint32_t db_size_t;

class AVLNode
{
public:
    db_key_t key;
    db_val_t val;
    AVLNode *left;
    AVLNode *right;
    db_size_t height;

    AVLNode(db_key_t key, db_val_t val);
    void put(db_key_t key, db_val_t val);
    db_val_t get(db_key_t key);
    void scan(
            db_key_t min_key,
            db_key_t max_key,
            vector<pair<db_key_t, db_val_t>> &pairs
    );
    AVLNode * rotate_left();
    AVLNode * rotate_right();
};

class AVLTree
{
public:
    AVLNode *root;

    AVLTree();
    void put(db_key_t key, db_val_t val);
    db_val_t get(db_key_t key);
    vector<pair<db_key_t, db_val_t>> scan(db_key_t min_key, db_key_t max_key);
};

class Memtable
{
public:
    db_size_t max_size;
    db_size_t size;
    AVLTree tree;

    Memtable(db_size_t memtable_size);
    void put(db_key_t key, db_val_t val);
    db_val_t get(db_key_t key);
    vector<pair<db_key_t, db_val_t>> scan(db_key_t min_key, db_key_t max_key);
};

#endif
#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <cstdint>
#include "avl_tree.h"

class Entry
{
public:
    Entry *left;
    Entry *right;
    Entry *parent;
    uint32_t key;
    uint32_t val;
};

class Memtable
{
public:
    Memtable(uint32_t memtable_size);
    uint64_t get(uint64_t key);
    void put(uint64_t key, uint64_t val);
    void scan(uint64_t min_key, uint64_t max_key);

private:
    uint32_t memtable_size;
    uint32_t current_size;
    Entry root;

    // helpers for recursion
    uint64_t get_helper(Entry root, uint64_t key);
    void put_helper(Entry root, uint64_t key, uint64_t val);
    void scan_helper(Entry root, uint64_t min_key, uint64_t max_key);
};

#endif
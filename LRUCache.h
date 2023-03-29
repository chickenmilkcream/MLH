//
// Created by oof on 3/13/2023.
//

#ifndef CSC443_PA_LRUCACHE_H
#define CSC443_PA_LRUCACHE_H

#include <unordered_map>
#include "bp_pageframe.h"
#include "bp_linkedlist.h"

class LRUCache {
public:
    LRUCache();
    void put(int key, shared_ptr<PageFrame> value);

    void print_list();
    struct CacheNode {
        int key;
        weak_ptr<CacheNode> prev;
        shared_ptr<CacheNode> next;
        shared_ptr<PageFrame> value;
        CacheNode(int k, shared_ptr<PageFrame> v) : key(k), prev(), next(nullptr), value(v){}
    };
    shared_ptr<CacheNode>get(int key);
    shared_ptr<CacheNode> head;
    weak_ptr<CacheNode> tail;
    void mark_item_as_used(shared_ptr<PageFrame> pageFrame);

    shared_ptr<PageFrame> evict_one_page_item();

private:
    void remove(shared_ptr<CacheNode> node);
    void moveToFront(shared_ptr<CacheNode> node);
    std::unordered_map<int, shared_ptr<CacheNode>> cache;

};

#endif /* CSC443_PA_LRUCACHE_H */

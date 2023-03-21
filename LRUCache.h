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
    void put(int key, PageFrame *value);

    void print_list();
    struct CacheNode {
        int key;
        CacheNode* prev;
        CacheNode* next;
        PageFrame* value;
        CacheNode(int k, PageFrame* v) : key(k), prev(nullptr), next(nullptr), value(v){}
    };
    CacheNode * get(int key);
    CacheNode* head;
    CacheNode* tail;
    void mark_item_as_used(PageFrame *pageFrame);

    PageFrame * evict_one_page_item();

private:
    void remove(CacheNode* node);
    void moveToFront(CacheNode* node);
    std::unordered_map<int, CacheNode*> cache;

};

#endif /* CSC443_PA_LRUCACHE_H */

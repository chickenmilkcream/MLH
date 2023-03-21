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
    LRUCache(int capacity);
    PageFrame *put(int key, PageFrame *value);

    void set_capacity(int val);

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
    void use_page(PageFrame *pageFrame);
private:

    void remove(CacheNode* node);
    void moveToFront(CacheNode* node);
    std::unordered_map<int, CacheNode*> cache;
    int capacity;


};

#endif /* CSC443_PA_LRUCACHE_H */

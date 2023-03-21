//
// Created by oof on 3/13/2023.
//

#include "LRUCache.h"
#include <iostream>
//using namespace std;


LRUCache::LRUCache(int capacity) : capacity(capacity), head(nullptr), tail(nullptr) {}

void LRUCache::use_page(PageFrame* pageFrame) {
    if (pageFrame == nullptr) {
        return;
    }
    int key = pageFrame->get_page_number();
//    cout << "using page with key: " << key << endl;
    auto it = cache.find(key);
    if (it == cache.end()) {
//        cout << "key not found" << key << endl;
        return; // key not found

    }

    CacheNode* node = it->second;
//    cout << "before move to front: " << endl;
//    print_list();
//    cout << "after move to front: " << endl;
    moveToFront(node);
}

void::LRUCache::set_capacity(int val) {
    this->capacity = val;
}

PageFrame * LRUCache::put(int key, PageFrame *value) {
    PageFrame* pageFrame = nullptr;
    auto it = cache.find(key);
    if (it != cache.end()) {
        CacheNode* node = it->second;
        node->value = value; // update value
        moveToFront(node);
    } else {
        if (cache.size() >= capacity) {
            cache.erase(tail->key);
            pageFrame = tail->value;
            remove(tail);
        }
        CacheNode* node = new CacheNode(key, value);
        cache[key] = node;
        if (head == nullptr) {
            head = node;
            tail = node;
        } else {
            node->next = head;
            head->prev = node;
            head = node;
        }
    }
    return pageFrame;
}

void LRUCache::remove(CacheNode* node) {
    if (node == head) {
        head = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node == tail) {
        tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    cout << "LRU evicting page number " << node->value->get_page_number() << endl;
    this->print_list();
    delete node;
}

void LRUCache::moveToFront(CacheNode* node) {
    if (node == head) {
        return;
    }
    if (node == tail) {
        tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    node->prev->next = node->next;
    node->prev = nullptr;
    node->next = head;
    head->prev = node;
    head = node;
}

void LRUCache::print_list() {
    CacheNode* node = head;
    cout << "lru cache: ";
    while (node != nullptr) {
        cout << node->value->get_page_number() << " ";
        node = node->next;
    }
    cout << endl;
}

LRUCache::CacheNode *LRUCache::get(int key) {
    return nullptr;
}


//int main() {
//    LRUCache cache(2);
//
//    // Test case 1: Add two elements to the cache and retrieve one of them
//    cache.put(1, 10);
//    cache.put(2, 20);
//    std::cout << "Value of key 1: " << cache.get(1) << std::endl; // Expected output: 10
//
//    // Test case 2: Add an element to the cache that exceeds the capacity
//    cache.put(3, 30);
//    std::cout << "Value of key 2: " << cache.get(2) << std::endl; // Expected output: -1
//
//    // Test case 3: Update an element in the cache
//    cache.put(2, 22);
//    std::cout << "Value of key 2: " << cache.get(2) << std::endl; // Expected output: 22
//
//    // Test case 4: Add an element to the cache and then remove it
//    cache.put(4, 40);
//    std::cout << "Value of key 1: " << cache.get(1) << std::endl; // Expected output: -1
//
//    return 0;
//}

#include "LRUCache.h"
#include <iostream>

LRUCache::LRUCache() :head(nullptr), tail() {}

void LRUCache::mark_item_as_used(shared_ptr<PageFrame> pageFrame)
{
    if (pageFrame == nullptr) {
        return;
    }
    int key = pageFrame->get_page_number();
    auto it = cache.find(key);
    if (it == cache.end()) {
        return; // key not found
    }

    shared_ptr<CacheNode> node = it->second;
    moveToFront(node);
}



shared_ptr<PageFrame> LRUCache::evict_one_page_item() {
    shared_ptr<CacheNode> shared_tail = tail.lock();
    if (!shared_tail) {
        return nullptr;
    }
    int key = shared_tail->key;
    shared_ptr<PageFrame> pageFrame = shared_tail->value;
    cache.erase(key);
    remove(shared_tail);
    return pageFrame;
}

void LRUCache::put(int key, shared_ptr<PageFrame> value) {
    auto it = cache.find(key);
    if (it != cache.end()) {
        shared_ptr<CacheNode> node = it->second;
        node->value = value; // update value
        moveToFront(node);
    } else {
//        if (cache.size() >= capacity) {
////            cache.erase(tail->key);
////            pageFrame = tail->value;
////            remove(tail);
//            evict_one_page_item();
//        }
        shared_ptr<CacheNode> node = make_shared<CacheNode>(key, value);
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
}

void LRUCache::remove(shared_ptr<CacheNode> node) {
    shared_ptr<CacheNode> shared_tail = tail.lock();
    shared_ptr<CacheNode> shared_prev = node->prev.lock();
    if (node == head) {
        head = node->next;
    } else {
        shared_prev->next = node->next;
    }
    if (node == shared_tail) {
        tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
}

void LRUCache::moveToFront(shared_ptr<CacheNode> node) {
    shared_ptr<CacheNode> shared_tail = tail.lock();
    shared_ptr<CacheNode> shared_prev = node->prev.lock();
    if (node == head) {
        return;
    }
    if (node == shared_tail) {
        tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    shared_prev->next = node->next;
    node->prev.reset();
    node->next = head;
    head->prev = node;
    head = node;
}

void LRUCache::print_list() {
    shared_ptr<CacheNode> node = head;
    cout << "lru cache: ";
    while (node != nullptr) {
        cout << node->value->get_page_number() << " ";
        node = node->next;
    }
    cout << endl;
}

shared_ptr<LRUCache::CacheNode> LRUCache::get(int key) {
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

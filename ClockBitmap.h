//
// Created by oof on 3/13/2023.
//

#ifndef CSC443_PA_CLOCKBITMAP_H
#define CSC443_PA_CLOCKBITMAP_H
#include <unordered_map>
#include "bp_pageframe.h"
#include "bp_linkedlist.h"
#include <vector>
#include <memory>

class ClockBitmap {
public:
    ClockBitmap(int capacity);
    shared_ptr<PageFrame>  get(int key);
    shared_ptr<PageFrame> put(int key, shared_ptr<PageFrame> value);
    void set_capacity(int val);
    void use_item(shared_ptr<PageFrame> pageFrame);
    vector<shared_ptr<PageFrame>> evict_pages();
    void print_bitmap();

private:
    int capacity;
    vector<shared_ptr<PageFrame>> bitmap;
    int clock_hand = 0;
};



#endif //CSC443_PA_CLOCKBITMAP_H

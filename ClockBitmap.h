//
// Created by oof on 3/13/2023.
//

#ifndef CSC443_PA_CLOCKBITMAP_H
#define CSC443_PA_CLOCKBITMAP_H
#include <unordered_map>
#include "bp_pageframe.h"
#include "bp_linkedlist.h"
#include <vector>

class ClockBitmap {
public:
    ClockBitmap(int capacity);
    PageFrame * get(int key);
    PageFrame *put(int key, PageFrame *value);
    void set_capacity(int val);
    void use_page(PageFrame* pageFrame);
    vector<PageFrame*> evict_pages();
    void print_bitmap();

private:
    int capacity;
    vector<PageFrame*> bitmap;
    int clock_hand = 0;
};



#endif //CSC443_PA_CLOCKBITMAP_H

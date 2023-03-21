//
// Created by oof on 3/13/2023.
//

#include "ClockBitmap.h"
#include <iostream>


ClockBitmap::ClockBitmap(int capacity) {
    this->capacity = capacity;
    for (int i = 0; i < capacity; i++) {
        this->bitmap.push_back(nullptr);
    }
}

PageFrame* ClockBitmap::get(int key) {
    // probably don't need this function...?
    return 0;
}

void ClockBitmap::set_capacity(int val) {
    // set the capacity of the cache to val
    if (this->capacity < val) {
        for (int i = 0; i < val - this->capacity; i++) {
            this->bitmap.push_back(nullptr);
        }
    } else if (this->capacity > val) {
        //TODO: evict pages
    }
    this->capacity = val;
}

PageFrame *ClockBitmap::put(int key, PageFrame *value) {
    // put a page into the cache and return the evicted page using clock eviction algorithm

    PageFrame* evicted_page = nullptr;
    while (true) {
        if (this->bitmap[clock_hand] == nullptr) {
            this->bitmap[clock_hand] = value;
            return nullptr;
        } else if (this->bitmap[clock_hand]->get_reference_bit() == 0) {
            evicted_page = this->bitmap[clock_hand];
            this->bitmap[clock_hand] = value;
            return evicted_page;
        } else {
            this->bitmap[clock_hand]->set_reference_bit(0);
            clock_hand = (clock_hand + 1) % this->capacity;
        }
    }
}


void ClockBitmap::use_item(PageFrame* pageFrame) {
    // set the reference bit of the page to 1
    pageFrame->set_reference_bit(1);
}


vector<PageFrame *> ClockBitmap::evict_pages() {
    // TODO this function only called during resize
    vector<PageFrame *>evicted_pages = vector<PageFrame*>();


    return evicted_pages;
}

void ClockBitmap::print_bitmap() {
    for (int i = 0; i < this->capacity; i++) {
        if (this->bitmap[i] == nullptr) {
            cout << "- ";
        } else {
            cout << this->bitmap[i]->get_page_number() << " ";
        }
    }
    cout << endl;
}

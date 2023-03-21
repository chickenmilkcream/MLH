#include "bp_pageframe.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>

#include <fcntl.h>

PageFrame::PageFrame(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number)
{
    this->page_content = page_content;
    this->sst_name = sst_name;
    this->page_number = page_number;
    this->num_pairs_in_page = num_pairs_in_page;
    next = nullptr;
    clock_bit = 1;
}

int PageFrame::get_reference_bit() {
    return clock_bit;
}

void PageFrame::set_reference_bit(int bit) {
    clock_bit = bit;
}

int PageFrame::get_id() {
    return id;
}

void PageFrame::set_id(int id) {
    this->id = id;
}

int PageFrame::get_page_number() {
    return page_number;
}



#ifndef BP_PAGEFRAME_H
#define BP_PAGEFRAME_H

#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include "BloomFilter.h"

using namespace std;

typedef int64_t db_key_t;
typedef int64_t db_val_t;

class PageFrame
{
public:
    PageFrame(void *page_content, int num_pairs_in_page, string sst_name, int page_number);
    string sst_name;
    int page_number;
    int num_pairs_in_page;
    void *page_content;
    shared_ptr<PageFrame> next;
    int clock_bit;
    int id;

    int get_reference_bit();
    void set_reference_bit(int bit);
    int get_id();
    void set_id(int id);
    int get_page_number();

    BloomFilter *bloomFilter;
};

#endif

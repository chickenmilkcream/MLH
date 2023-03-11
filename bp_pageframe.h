#ifndef BP_PAGEFRAME_H
#define BP_PAGEFRAME_H

#include <cstdint>
#include <string>
#include <iostream>

using namespace std;

typedef int64_t db_key_t;
typedef int64_t db_val_t;

class PageFrame
{
public:
    PageFrame(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number);
    string sst_name;
    int page_number;
    int num_pairs_in_page;
    pair<db_key_t, db_val_t> *page_content;
    PageFrame *next;
};

#endif

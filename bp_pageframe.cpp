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
}
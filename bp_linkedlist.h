#ifndef BP_LINKEDLIST_H
#define BP_LINKEDLIST_H

#include "bp_pageframe.h"

#include <memory>

using namespace std;

class BPLinkedList
{
public:
    shared_ptr<PageFrame> head;
    int size;
    BPLinkedList();
    shared_ptr<PageFrame> add_page_frame(void *page_content, int num_pairs_in_page, string sst_name, int page_number);
    shared_ptr<PageFrame> find_page_frame(string sst_name, int page_number);
    void remove_page_frame(shared_ptr<PageFrame> page_frame);
    void print_list();
    int free_all_pages();
    int clock_cycle_count;
};

#endif

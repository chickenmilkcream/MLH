#ifndef BP_LINKEDLIST_H
#define BP_LINKEDLIST_H

#include "bp_pageframe.h"

using namespace std;

class BPLinkedList
{
public:
    PageFrame *head;
    int size;
    BPLinkedList();
    PageFrame* add_page_frame(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number);
    PageFrame* find_page_frame(string sst_name, int page_number);
    void remove_page_frame(PageFrame *page_frame);
    void print_list();
    void free_all_pages();
};

#endif

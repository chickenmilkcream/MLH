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
    void add_page_frame(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number);
    pair<db_key_t, db_val_t> *find_page_frame(string sst_name, int page_number);
    void print_list();
};

#endif

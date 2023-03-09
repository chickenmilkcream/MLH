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
    void add_page_frame(int page_content, string sst_name, int page_number);
    int find_page_frame(string sst_name, int page_number);
    void print_list();
};

#endif

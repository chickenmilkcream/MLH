#ifndef BP_LINKEDLIST_H
#define BP_LINKEDLIST_H

#include "bp_pageframe.h"

using namespace std;

class BPLinkedList
{
public:
    PageFrame *head;

    BPLinkedList();
    void addPageFrame(int val);
    void printList();
};

#endif

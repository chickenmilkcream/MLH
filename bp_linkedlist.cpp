#include "bp_linkedlist.h"

BPLinkedList::BPLinkedList()
{
    head = nullptr;
}

void BPLinkedList::addPageFrame(int val)
{
    PageFrame *newNode = new PageFrame(val);
    if (head == nullptr)
    {
        head = newNode;
    }
    else
    {
        PageFrame *current = head;
        while (current->next != nullptr)
        {
            current = current->next;
        }
        current->next = newNode;
    }
}

void BPLinkedList::printList()
{
    PageFrame *current = head;
    while (current != nullptr)
    {
        std::cout << current->value << " ";
        current = current->next;
    }
    std::cout << std::endl;
}

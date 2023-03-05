#include "bp_linkedlist.h"

BPLinkedList::BPLinkedList()
{
    head = nullptr;
    size = 0;
}

void BPLinkedList::addPageFrame(int val)
{
    this->size ++;
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
    std::cout << "The size: " << this->size << std::endl;
    std::cout << "The linkedlist: " << std::endl;

    while (current != nullptr)
    {
        std::cout << current->value << " ";
        current = current->next;
    }
    std::cout << std::endl;
}

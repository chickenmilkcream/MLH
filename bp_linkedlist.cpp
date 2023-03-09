#include "bp_linkedlist.h"

BPLinkedList::BPLinkedList()
{
    head = nullptr;
    size = 0;
}

void BPLinkedList::add_page_frame(int page_content, string sst_name, int page_number)
{
    this->size ++;

    PageFrame *newNode = new PageFrame(page_content, sst_name, page_number);
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

int BPLinkedList::find_page_frame(string sst_name, int page_number)
{
    PageFrame *current = head;
    while (current != nullptr)
    {
        if (current->sst_name == sst_name && current->page_number == page_number) {
            return current->page_content;
        }
        current = current->next;
    }

    throw out_of_range("Couldn't find page frame in linkedlist");
}

void BPLinkedList::print_list()
{
    PageFrame *current = head;
    std::cout << "The linkedlist: " << std::endl;

    while (current != nullptr)
    {
        std::cout << current->page_content << " ";
        current = current->next;
    }
    std::cout << std::endl;
}

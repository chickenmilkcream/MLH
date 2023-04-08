#include "bp_linkedlist.h"
#include "memory"
#include <vector>
BPLinkedList::BPLinkedList()
{
    head = nullptr;
    size = 0;
    clock_cycle_count = 0;
}

shared_ptr<PageFrame> BPLinkedList::add_page_frame(void *page_content, int num_pairs_in_page, string sst_name, int page_number)
{
    this->size ++;

    shared_ptr<PageFrame> newNode = make_shared<PageFrame>(page_content, num_pairs_in_page, sst_name, page_number);
    if (head == nullptr)
    {
        head = newNode;
    }
    else
    {
        shared_ptr<PageFrame> current = head;
        while (current->next != nullptr)
        {
            current = current->next;
        }
        current->next = newNode;
    }
    return newNode;
}

shared_ptr<PageFrame> BPLinkedList::find_page_frame(string sst_name, int page_number)
{
    shared_ptr<PageFrame> current = head;
    while (current != nullptr)
    {
        if (current->sst_name == sst_name && current->page_number == page_number) {

            return current;
        }
        current = current->next;
    }

    throw out_of_range("Couldn't find page frame in linkedlist");
}


void BPLinkedList::print_list()
{
    shared_ptr<PageFrame> current = head;
    while (current != nullptr)
    {
        cout << current->sst_name << " Page " << current->page_number << ";" << endl;
        current = current->next;
    }
    cout << endl;
}

int BPLinkedList::free_all_pages()
{
    int num_pages_freed = 0;
    shared_ptr<PageFrame> current = head;
    while (current != nullptr)
    {
        num_pages_freed ++;
        shared_ptr<PageFrame> next = current->next;
        free(current->page_content);
        current = next;
    }
    return num_pages_freed;
}

void BPLinkedList::remove_page_frame(shared_ptr<PageFrame> page_frame) {
    shared_ptr<PageFrame> current = head;
    shared_ptr<PageFrame> prev = nullptr;

    while (current != nullptr) {
        if (current->page_number == page_frame->page_number) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }

            free(current->page_content);
            return;
        }
        prev = current;
        current = current->next;
    }

}

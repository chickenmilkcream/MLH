#include "bp_linkedlist.h"
#include "memory"

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
    // PageFrame *current = head;
    // std::cout << "The linkedlist: " << std::endl;
    // std::cout << current << std::endl;

    // while (current != nullptr)
    // {
    //     cout << "--- start page" << endl;
    //     for (int i = 0; i < current->num_pairs_in_page; i++)
    //     {
    //         std::cout << "Key: " << current->page_content[i].first << " Value: " << current->page_content[i].second << " Clock Reference bit: " << current->get_reference_bit() << std::endl;
    //     }
    //     cout << "--- end page" << endl;

    //     current = current->next;
    // }
    // std::cout << std::endl;
}

void BPLinkedList::free_all_pages()
{
    head = nullptr;
    shared_ptr<PageFrame> current = head;
    while (current != nullptr)
    {
        shared_ptr<PageFrame> next = current->next;
        free(current->page_content);
        // for (int i = 0; i < current->num_pairs_in_page; ++i)
        // {
        //     current->page_content[i].~pair<db_key_t, db_val_t>();
        // }
        // free(current);
        current = next;
    }
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
            // free(current);

            free(current->page_content);
            // // free the memory when you're done
            // for (int i = 0; i < current->num_pairs_in_page; ++i)
            // {
            //     current->page_content[i].~pair<db_key_t, db_val_t>();
            // }
            // delete current->page_content;

            // free(current);
            return;
        }
        prev = current;
        current = current->next;
    }

}

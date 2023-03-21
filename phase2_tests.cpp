#include <iostream>
#include <cassert>
#include "bp_directory.h"

int main(int argc, char *argv[])
{
    string sst_name = "sst_1.bin";
    int page_number = 1;
    string eviction_policy = "LRU";
    int initial_num_bits = 2;
    int maximum_bp_size = 4;
    int maximum_num_items_threshold = 10;
    int num_pairs_in_page = 1; // ideally, I would like to have more pairs, but I don't know how to do that with vector<pair<db_key_t, db_val_t>* >, the memory thing when I try to access page_content[i] gets weird...

    vector<pair<db_key_t, db_val_t> > page_content;
    for (auto i = 0; i != 12; ++i)
    {
        page_content.push_back(make_pair(i, i));
    }
    

    std::cout << "Running test cases for Phase 2 \n";
    std::cout << "--------------------------------------------------------------- \n";

    BPDirectory bpd = BPDirectory(eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold);

    assert(bpd.directory.size() == initial_num_bits^2);
    assert(bpd.current_num_items == 0);

    bpd.print_directory();

    std::cout << "* Function generate_binary_strings passed \n";
    std::cout << "* Create directory passed \n";
    std::cout << "--------------------------------------------------------------- \n";

    for (auto entry = 0; entry != 8; ++entry)
    {
        bpd.insert_page(&page_content[entry], num_pairs_in_page, sst_name, page_number);
        page_number++;
    }

    bpd.print_directory();
    assert(bpd.current_num_items == 8);
    assert(bpd.directory["00"]->size == 3);
    assert(bpd.directory["01"]->size == 1);
    assert(bpd.directory["10"]->size == 3);
    assert(bpd.directory["11"]->size == 1);

    std::cout << "* Function hash_string passed \n";
    std::cout << "* Add several page frames passed \n";
    std::cout << "--------------------------------------------------------------- \n";

    bpd.insert_page(&page_content[8], num_pairs_in_page, sst_name, page_number);

    page_number++;
    bpd.insert_page(&page_content[9], num_pairs_in_page, sst_name, page_number);
    bpd.print_directory();

    assert(bpd.current_num_bits == initial_num_bits + 1);
    assert(bpd.maximum_num_items_threshold == maximum_num_items_threshold * 2);
    assert(bpd.current_num_items == 10);
    assert(bpd.directory["000"]->size == 1);
    assert(bpd.directory["001"]->size == 3);
    assert(bpd.directory["010"]->size == 1);
    assert(bpd.directory["011"]->size == 1);
    assert(bpd.directory["100"]->size == 3);
    assert(bpd.directory["101"]->size == 1);
    assert(bpd.directory["110"]->size == 1);
    assert(bpd.directory["111"]->size == 1);
    assert(bpd.directory["010"] == bpd.directory["011"]);

    // std::cout << "* Expanding directory entries passed when max page number reached \n";
    std::cout << "* Finding entries sharing linkedlists passed when rehashing long linkedlists \n";
    std::cout << "* Rehashing long linkedlists passed when expanding directory entries \n";
    std::cout << "* Function get_keys_sharing_linkedlist passed \n";
    std::cout << "* Function rehash_linked_list passed \n";
    std::cout << "* Function extend_directory passed \n";
    std::cout << "--------------------------------------------------------------- \n";

    page_number++;
    bpd.insert_page(&page_content[10], num_pairs_in_page, sst_name, page_number);
    page_number++;
    bpd.insert_page(&page_content[11], num_pairs_in_page, sst_name, page_number);
    bpd.print_directory();

    assert(bpd.directory["010"]->size == 0);
    assert(bpd.directory["011"]->size == 2);

    std::cout << "* Rehashing long linkedlists passed when inserting a page to an entry with shared linklists \n";
    std::cout << "* Function insert_page passed \n";
    std::cout << "--------------------------------------------------------------- \n";

    assert(bpd.get_page(sst_name, 1)->page_content->first == 0);
    assert(bpd.get_page(sst_name, 2)->page_content->first == 1);
    assert(bpd.get_page(sst_name, 3)->page_content->first == 2);
    assert(bpd.get_page(sst_name, 4)->page_content->first == 3);
    assert(bpd.get_page(sst_name, 5)->page_content->first == 4);
    assert(bpd.get_page(sst_name, 6)->page_content->first == 5);
    assert(bpd.get_page(sst_name, 7)->page_content->first == 6);
    assert(bpd.get_page(sst_name, 8)->page_content->first == 7);
    assert(bpd.get_page(sst_name, 9)->page_content->first == 8);
    assert(bpd.get_page(sst_name, 10)->page_content->first == 9);
    assert(bpd.get_page(sst_name, 11)->page_content->first == 10);
    assert(bpd.get_page(sst_name, 12)->page_content->first == 11);

    try
    { // making sure that out_of_range is thrown when trying to retrieve non page
        bpd.get_page(sst_name, 13);
        assert(false);
    }
    catch (out_of_range e)
    {
    }

    std::cout << "* Function get_page passed \n";
    std::cout << "--------------------------------------------------------------- \n";

    // TODO AMY: test edit directory size
    // TODO JASON: test calling evict_directory during extend_directory with different policies
    // TODO JUN: test b tree stuff, might have to edit phase 1 tests for that
}

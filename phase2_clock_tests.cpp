//
// Created by oof on 3/14/2023.
//
//
// Created by oof on 3/12/2023.
//
#include <iostream>
#include <cassert>
#include "bp_directory.h"

int main(int argc, char *argv[]) {
    string sst_name = "sst_1.bin";
    int page_number = 1;
    string eviction_policy = "clock";
    int initial_num_bits = 2;
    int maximum_bp_size = 4;
    int maximum_num_items_threshold = 10;
    int num_pairs_in_page = 1; // ideally, I would like to have more pairs, but I don't know how to do that with vector<pair<db_key_t, db_val_t>* >, the memory thing when I try to access page_content[i] gets weird...

    vector<pair<db_key_t, db_val_t> > page_content;
    for (auto i = 0; i != 64; ++i) {
        page_content.push_back(make_pair(i, i));
    }

//
//    std::cout << "Running test cases for Phase 2 \n";
//    std::cout << "--------------------------------------------------------------- \n";

    BPDirectory bpd = BPDirectory(eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold);


    assert(bpd.directory.size() == initial_num_bits ^ 2);
    assert(bpd.current_num_items == 0);

//    bpd.print_directory();

//    std::cout << "* Function generate_binary_strings passed \n";
//    std::cout << "* Create directory passed \n";
//    std::cout << "--------------------------------------------------------------- \n";

    for (auto entry = 0; entry != 8; ++entry) {
        bpd.insert_page(&page_content[entry], num_pairs_in_page, sst_name, page_number);
        page_number++;
    }

//    bpd.print_directory();
    assert(bpd.current_num_items == 8);
    assert(bpd.directory["00"]->size == 3);
    assert(bpd.directory["01"]->size == 1);
    assert(bpd.directory["10"]->size == 3);
    assert(bpd.directory["11"]->size == 1);

//    std::cout << "* Function hash_string passed \n";
//    std::cout << "* Add several page frames passed \n";
//    std::cout << "--------------------------------------------------------------- \n";

    bpd.insert_page(&page_content[8], num_pairs_in_page, sst_name, page_number);

    page_number++;
    bpd.insert_page(&page_content[9], num_pairs_in_page, sst_name, page_number);
//    bpd.print_directory();

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



    std::cout << "passed amy's tests. this is not a woman moment \n testing eviction now    \n";

    bpd.set_maximum_num_items(16);


//    bpd.mark_item_as_used()

    for (auto i = 10; i != 16; i++) {
        page_number++;
        bpd.insert_page(&page_content[i], num_pairs_in_page, sst_name, page_number);
    }

    // use pages 2-4 to test eviction
    for (auto i = 2; i != 5; i++) {
        bpd.get_page(sst_name, i);
    }

    // insert 3 more pages to test eviction
    // now page 1 should get evicted but not 2-4
    for (auto i = 16; i != 19; i++) {
        page_number++;
        bpd.insert_page(&page_content[i], num_pairs_in_page, sst_name, page_number);
    }

//    assert(bpd.get_page(sst_name, 1) == nullptr);
//    assert(bpd.get_page(sst_name, 2) != nullptr);
//    assert(bpd.get_page(sst_name, 3) != nullptr);
//    assert(bpd.get_page(sst_name, 4) != nullptr);
//    assert(bpd.get_page(sst_name, 5) == nullptr);
//    assert(bpd.get_page(sst_name, 6) == nullptr);
//    assert(bpd.get_page(sst_name, 7) != nullptr);

    std::cout << "passed eviction tests \n";

    return 0;

}
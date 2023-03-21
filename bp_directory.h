#ifndef BP_DIRECTORY_H
#define BP_DIRECTORY_H

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <bitset>
#include "xxhash.h"
#include "bp_linkedlist.h"
#include "LRUCache.h"
#include "ClockBitmap.h"

using namespace std;

class BPDirectory
{
public:
    BPDirectory(string eviction_policy = "LRU", int initial_num_bits=0, int maximum_num_bits=0, int maximum_num_pages=0);
    void insert_page(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number);
    PageFrame* get_page(string sst_name, int page_number);
    void extend_directory();
    void evict_directory();
    void edit_directory_size(int new_maximum_num_bits);
    void set_policy(string policy);
    void rehash_linked_list(map<string, shared_ptr<BPLinkedList> > *directory, string key, vector<string> shared_keys);
    void print_directory();
    vector<string> get_keys_sharing_linkedlist(map<string, shared_ptr<BPLinkedList> > directory, string key);
    string hash_string(string source);
    void free_all_pages();
    
    string policy;
    int initial_num_bits;
    int current_num_bits;
    int maximum_num_bits;

    int current_num_pages;
    int maximum_num_pages;
    int absolute_maximum_num_pages;
    map<string, shared_ptr<BPLinkedList> > directory;

    void set_max_num_pages(int max_num_pages);
    shared_ptr<LRUCache> lru_cache;
    int page_id;
    shared_ptr<ClockBitmap> clock_bitmap;

    void use_page(PageFrame *pageFrame);

private:
    void evict_directory_lru();
    void evict_directory_clock();
    vector<string> generate_binary_strings(int n, string str = "");

    void evict_page(PageFrame* pageToEvict);


};

#endif

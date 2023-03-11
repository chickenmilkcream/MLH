#ifndef BP_DIRECTORY_H
#define BP_DIRECTORY_H

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "xxhash.h"
#include "bp_linkedlist.h"

using namespace std;

class BPDirectory
{
public:
    BPDirectory(string eviction_policy = "clock", int initial_num_bits = 0, int maximum_num_bits = 0, int maximum_num_pages = 0);
    void insert_page(int page, string sst_name, int page_number);
    int get_page(string sst_name, int page_number);
    void extend_directory();
    void evict_directory();
    void shrink_directory(int new_maximum_num_bits);
    void set_policy(string policy);
    void rehash_linked_list(map<string, shared_ptr<BPLinkedList> > *directory, string key, vector<string> shared_keys);
    void print_directory();
    vector<string> get_keys_sharing_linkedlist(map<string, shared_ptr<BPLinkedList> > directory, string key);
    string hash_string(string source);
    
    string policy;
    int initial_num_bits;
    int current_num_bits;
    int maximum_num_bits;

    int current_num_pages;
    int maximum_num_pages;
    map<string, shared_ptr<BPLinkedList> > directory;

private:
    void evict_directory_lru();
    void evict_directory_clock();
    vector<string> generate_binary_strings(int n, string str = "");
};

#endif

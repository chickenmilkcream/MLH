#ifndef BP_DIRECTORY_H
#define BP_DIRECTORY_H

#include <cstdint>
#include <string>
#include <map>
#include <memory>

#include "xxhash.h"
#include "bp_linkedlist.h"

using namespace std;

class BPDirectory
{
public:
    BPDirectory(string eviction_policy, int initial_num_bits, int maximum_num_bits, int maximum_num_pages);
    void insert_page(string sst_name, int page_number);
    void extend_directory();
    void evict_directory();
    void edit_directory_size(int new_num_bits = 0);
    void set_policy(string policy);

    map<string, shared_ptr<BPLinkedList> > directory;

private:
    string policy;
    int initial_num_bits;
    int current_num_bits;
    int maximum_num_bits;

    int current_num_pages;
    int maximum_num_pages;

    void evict_directory_lru();
    void evict_directory_clock();
    string hash_string(string source);
    vector<string> generate_binary_strings(int n, string str = "");
};

#endif

#ifndef BP_DIRECTORY_H
#define BP_DIRECTORY_H

#include <cstdint>
#include <string>
#include <map>

// #include "xxhash.h"
#include "bp_linkedlist.h"

using namespace std;

class BPDirectory
{
public:
    BPDirectory(string eviction_policy, int initial_num_bits, int maximum_num_bits, int maximum_num_pages);
    void insert_page(string db);
    void extend_directory();
    void evict_directory();
    void edit_directory_size(int new_size = 0);
    void set_policy(string policy);

    map<string, BPLinkedList> directory;

private:
    string policy;
    int initial_num_bits;
    int current_num_bits;
    int maximum_num_bits;

    int current_num_pages;
    int maximum_num_pages;

    void evict_directory_lru();
    void evict_directory_clock();
    vector<string> generateBinaryStrings(int n, string str = "");
};

#endif

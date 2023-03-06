#include "bp_directory.h"
#include "xxhash.h"

#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

using namespace std;

BPDirectory::BPDirectory(string eviction_policy, int initial_num_bits, int maximum_num_bits, int maximum_num_pages)
{
    this->policy = eviction_policy;

    this->initial_num_bits = initial_num_bits;
    this->current_num_bits = initial_num_bits;
    this->maximum_num_bits = maximum_num_bits;

    this->current_num_pages = 0;
    this->maximum_num_pages = maximum_num_pages;

    for (const string &prefix : generate_binary_strings(this->initial_num_bits))
    {
        this->directory[prefix] = make_shared<BPLinkedList>();
    }
}

void BPDirectory::insert_page(string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    // TODO AMY: hash actual 4kb page
    this->directory[directory_key]->addPageFrame(page_number);
    this->current_num_pages ++;

    if (this->current_num_pages > this->maximum_num_pages) {
        this->extend_directory();
    }
}

void BPDirectory::extend_directory()
{

    this->current_num_bits ++;
    this->maximum_num_pages *= 2;

    if (this->current_num_bits >= this->maximum_num_bits) {
        // TODO JASON: clarify when it will evict until?
        this->evict_directory();
    }

    // Double the size of the directory and make it point to the same linkedlist
    map<string, shared_ptr<BPLinkedList> > new_directory;
    for (const auto &pair : this->directory)
    {
        string key = pair.first;
        shared_ptr<BPLinkedList> value = pair.second;
        new_directory[key + "0"] = value;
        new_directory[key + "1"] = value;
    }
    this->directory = new_directory;

    for (const auto &pair : this->directory)
    {
        // TODO AMY: loop throguh linkedlist and rehash everything 
        if (pair.second->size > 1) {
            std::cout << pair.first << "TODO REHASH" << endl;
        }
    }
}

void BPDirectory::evict_directory() {
    if (this->policy.compare("LRU") == 0) {
        this->evict_directory_lru();
    } else if (this->policy.compare("Clock") == 0) {
        this->evict_directory_clock();
    } else {
        cout << "There is not a valid policy in place. Please set policy as LRU or Clock." << endl;
    }
}

void BPDirectory::evict_directory_lru()
{
    cout << "TODO JASON LRU" << endl;
}

void BPDirectory::evict_directory_clock()
{
    cout << "TODO JASON Clock" << endl;
}

vector<string> BPDirectory::generate_binary_strings(int n, string str)
{
    vector<string> result;
    if (n == 0)
    {
        result.push_back(str);
    }
    else
    {
        vector<string> v1 = generate_binary_strings(n - 1, str + "0");
        result.insert(result.end(), v1.begin(), v1.end());
        vector<string> v2 = generate_binary_strings(n - 1, str + "1");
        result.insert(result.end(), v2.begin(), v2.end());
    }
    return result;
}

string BPDirectory::hash_string(string source)
{
    uint64_t int_hash = XXH64(source.data(), source.size(), 0);
    string binary_hash = bitset<64>(int_hash).to_string();
    string directory_key = binary_hash.substr(0, this->current_num_bits);

    return directory_key;
}

void BPDirectory::set_policy(string policy) {
    if (policy.compare("LRU") != 0 || policy.compare("Clock") != 0) {
        cout << "This is not a valid policy. Please pass in LRU or Clock." << endl;
    } else {
        this->policy = policy;
    }
}

void BPDirectory::edit_directory_size(int new_num_bits) {

    if (new_num_bits > this->current_num_bits) {
        // TODO TEAM: figure out confusion with max_size being actual size or number of prefix bits ahh
        // TODO TEAM: figure out what to do here 
        this->evict_directory();
        // TODO AMY: supposed to implement a shrink_directory() as well
    }

    this->current_num_bits = new_num_bits;
    this->maximum_num_bits = new_num_bits;
}
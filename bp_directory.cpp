#include "bp_directory.h"

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
    this->current_num_bits = current_num_bits;
    this->maximum_num_bits = maximum_num_bits;

    this->current_num_pages = 0;
    this->maximum_num_pages = maximum_num_pages;

    for (const string &prefix : generateBinaryStrings(this->initial_num_bits)) {
        this->directory[prefix] = make_shared<BPLinkedList>();
    }
}

void BPDirectory::extend_directory() {

    this->current_num_bits ++;

    if (this->current_num_bits >= this->maximum_num_bits) {
        // TODO: Ask when it will evict until?
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
    cout << "TODO LRU" << endl;
}

void BPDirectory::evict_directory_clock()
{
    cout << "TODO Clock" << endl;
}

vector<string> BPDirectory::generateBinaryStrings(int n, string str)
{
    vector<string> result;
    if (n == 0)
    {
        result.push_back(str);
    }
    else
    {
        vector<string> v1 = generateBinaryStrings(n - 1, str + "0");
        result.insert(result.end(), v1.begin(), v1.end());
        vector<string> v2 = generateBinaryStrings(n - 1, str + "1");
        result.insert(result.end(), v2.begin(), v2.end());
    }
    return result;
}

void BPDirectory::set_policy(string policy) {
    if (policy.compare("LRU") != 0 || policy.compare("Clock") != 0) {
        cout << "This is not a valid policy. Please pass in LRU or Clock." << endl;
    } else {
        this->policy = policy;
    }
}

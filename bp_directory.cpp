#include "bp_directory.h"

#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>

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
        BPLinkedList linkedList;
        this->directory[prefix] = linkedList;
    }
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
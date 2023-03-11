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

void BPDirectory::insert_page(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    // Malloc memory for the page
    pair<db_key_t, db_val_t> *malloc_page = (pair<db_key_t, db_val_t> *)malloc(num_pairs_in_page * sizeof(pair<db_key_t, db_val_t>));
    for (int i = 0; i < num_pairs_in_page; ++i)
    {
        db_key_t key = page_content[i].first;
        db_val_t val = page_content[i].second;
        new (&malloc_page[i]) pair<db_key_t, db_val_t>(key, val);
    }

    // // NOTE to Jason: this is how to free the memory, could be useful for eviction stuff
    // // free the memory when you're done
    // for (int i = 0; i < num_pairs_in_page; ++i)
    // {
    //     malloc_page[i].~pair<db_key_t, db_val_t>();
    // }
    // free(malloc_page);

    this->directory[directory_key]->add_page_frame(malloc_page, num_pairs_in_page, sst_name, page_number);
    this->current_num_pages += 1;

    if (this->current_num_pages >= this->maximum_num_pages) {
        // This should rehash all the linkedlists greater than length 1 after expansion
        this->extend_directory();
        return;
    }

    // Rehash as soon as chain length gets to be greater than 1 
    // Retrieved from: https://piazza.com/class/lckjb9lrfaa57i/post/78
    // Case 1: a bucket pointed at by just one pointer from the directory. This bucket already expanded since the last time the directory expanded. We need to patiently wait until the directory expands again before we can expand this bucket.
    // Case 2: a bucket pointed at by more than one pointer from the directory. We should expand this bucket immediately when it overflows.
    // This is Case 2
    vector<string> shared_keys = this->get_keys_sharing_linkedlist(this->directory, directory_key);
    if (this->directory[directory_key]->size > 1 && shared_keys.size() > 1) {
        this->rehash_linked_list(&this->directory, directory_key, shared_keys);
    }
}

pair<db_key_t, db_val_t> *BPDirectory::get_page(string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    return this->directory[directory_key]->find_page_frame(sst_name, page_number);
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
        string entry = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        new_directory[entry + "0"] = linkedlist;
        new_directory[entry + "1"] = linkedlist;

        if (linkedlist->size > 1)
        {
            vector<string> shared_keys = this->get_keys_sharing_linkedlist(new_directory, entry + "0"); // Shared keys should only be entry + "0" and entry + "1"
            this->rehash_linked_list(&new_directory, entry + "0", shared_keys);
        }
    }
    this->directory = new_directory;
    }

void BPDirectory::evict_directory() {
    // TODO JASON: might have to modify current_num_bits and max_num_pages and current_num_pages
    if (this->policy.compare("LRU") == 0) {
        this->evict_directory_lru();
    } else if (this->policy.compare("clock") == 0) {
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

void BPDirectory::shrink_directory(int new_maximum_num_bits) {

    if (new_maximum_num_bits >= this->maximum_num_bits) {
        cout << "Please the new maximum num bits to be less than the current maximum num bits: " << this->maximum_num_bits << endl;
        return;
    }

    if (new_maximum_num_bits < this->current_num_bits)
    {
        // TODO TEAM: figure out confusion with max_size being actual size or number of prefix bits ahh
        // TODO TEAM: figure out what to do here

        // Evict extra entries, but which ones? How many times do we evict? current_num_bits - new_maximum_num_bits??
        // When do we stop evicting? -> what do we set current_num_bits to be?
        this->evict_directory();
        this->current_num_bits = new_maximum_num_bits;

        // TODO AMY: decrement max_num_pages several times, should be taken care of in evict_directory
        // TODO AMY: shrink the actual directory table hmmmmmm so confused here
    }

    this->maximum_num_bits = new_maximum_num_bits;
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
    if (policy.compare("LRU") != 0 || policy.compare("clock") != 0) {
        throw invalid_argument("This is not a valid policy. Please pass in LRU or clock.");
    } else {
        this->policy = policy;
    }
}

void BPDirectory::rehash_linked_list(map<string, shared_ptr<BPLinkedList> > *directory, string key, vector<string> shared_keys)
{
    // Right now we have a situation where multiple entries both point to the same linkedlist
    // but this linkedlist need to be rehashed between the multiple entries because the length is too long
    map<string, shared_ptr<BPLinkedList> > key_to_new_linkedlist;

    for (const auto &shared_key : shared_keys)
    {
        key_to_new_linkedlist[shared_key] = make_shared<BPLinkedList>();
    }

    PageFrame *current = (*directory)[key]->head;
    while (current != nullptr)
    {
        string source = current->sst_name + to_string(current->page_number);
        string directory_key = this->hash_string(source);
        key_to_new_linkedlist[directory_key]->add_page_frame(current->page_content, current->num_pairs_in_page, current->sst_name, current->page_number);
        current = current->next;
    }

    for (const auto &pair : key_to_new_linkedlist)
    {
        string entry = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        (*directory)[entry] = linkedlist;
    }
}

vector<string> BPDirectory::get_keys_sharing_linkedlist(map<string, shared_ptr<BPLinkedList> > directory, string key)
{
    vector<string> shared_keys;
    shared_ptr<BPLinkedList> shared_linkedlist = directory[key];

    for (const auto &pair : directory)
    {
        string entry = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;

        if (linkedlist == shared_linkedlist)
        {
            shared_keys.push_back(entry);
        }
    }

    return shared_keys;
}

void BPDirectory::print_directory() {
    cout << "The number of pages in the directory: " << this->current_num_pages << endl;
    for (auto entry = this->directory.begin(); entry != this->directory.end(); ++entry)
    {
        std::cout << "The directory entry: " << entry->first << std::endl;
        entry->second->print_list();
    }
}

void BPDirectory::free_all_pages()
{
    for (auto entry = this->directory.begin(); entry != this->directory.end(); ++entry)
    {
        entry->second->free_all_pages();
    }
}

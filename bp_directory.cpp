#include "bp_directory.h"
#include "xxhash.h"
#include "LRUCache.h"
#include "bp_pageframe.h"
#include "ClockBitmap.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

using namespace std;

BPDirectory::BPDirectory(string eviction_policy, int initial_num_bits, int maximum_bp_size, int maximum_num_items_threshold)
{
    this->policy = eviction_policy;

    this->initial_num_bits = initial_num_bits;
    this->current_num_bits = initial_num_bits;

    this->current_bp_size = 0;
    this->maximum_bp_size = maximum_bp_size; // This is in terms of the max number of bytes that the buffer pool can contain before eviction policy should kick in
    
    this->current_num_items = 0;
    this->maximum_num_items_threshold = maximum_num_items_threshold;
    this->page_id = 0; // the total number of pages we've interacted before (placeholder value)

    for (const string &prefix : generate_binary_strings(this->initial_num_bits))
    {
        this->directory[prefix] = make_shared<BPLinkedList>();
    }

    this->lru_cache = make_shared<LRUCache>(this->maximum_num_items_threshold + 10000); // TODO get rid of this param
    this->clock_bitmap = make_shared<ClockBitmap>(this->maximum_num_items_threshold + 10000);
}

void BPDirectory::set_maximum_num_items(int value)
{
    this->maximum_num_items_threshold = value;
    if (this->policy == "LRU") {
        this->lru_cache->set_capacity(value);
    }
    else {
        this->clock_bitmap->set_capacity(value);
    }
}

void BPDirectory::insert_page(pair<db_key_t, db_val_t> *page_content, int num_pairs_in_page, string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);
    this->current_bp_size += sizeof(PageFrame);

    // Malloc memory for the page
    pair<db_key_t, db_val_t> *malloc_page = (pair<db_key_t, db_val_t> *)malloc(num_pairs_in_page * sizeof(pair<db_key_t, db_val_t>));
    for (int i = 0; i < num_pairs_in_page; ++i)
    {
        db_key_t key = page_content[i].first;
        db_val_t val = page_content[i].second;
        new (&malloc_page[i]) pair<db_key_t, db_val_t>(key, val);
    }

    PageFrame* newNode = this->directory[directory_key]->add_page_frame(malloc_page, num_pairs_in_page, sst_name, page_number);
    this->current_num_items += 1;
    this->page_id += 1;
    newNode->set_id(this->page_id);

    // TODO
    // if current_bp_size > max_bp_size: 
    // call evict api and update the current_bp_size until it's under the max_bp_size

    PageFrame *pageToEvict;
    if (this->policy == "LRU") {
        pageToEvict = this->lru_cache->put(newNode->page_number, newNode);
    }
    else { // clock
        pageToEvict = this->clock_bitmap->put(this->page_id, newNode);
    }
    if (pageToEvict != nullptr) {
        this->current_num_items--;
        this->current_bp_size -= sizeof(PageFrame);
        this->evict_page(pageToEvict);
//        this->clock_bitmap->print_bitmap();
    }

    if (this->current_num_items >= this->maximum_num_items_threshold) {
        // This should rehash all the linkedlists greater than length 1 after expansion
        this->extend_directory();
        return;
    } else {
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

void BPDirectory::use_item(PageFrame* pageFrame) {
    if (this->policy == "LRU") {
        this->lru_cache->use_item(pageFrame);
    }
    else { // clock
        this->clock_bitmap->use_item(pageFrame);
    }
}

PageFrame* BPDirectory::get_page(string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    PageFrame* pageFrame = this->directory[directory_key]->find_page_frame(sst_name, page_number);
    this->use_item(pageFrame);
    return pageFrame;
}

void BPDirectory::extend_directory()
{
    this->current_num_bits ++;
    this->maximum_num_items_threshold *= 2;

    // Double the size of the directory and make it point to the same linkedlist
    map<string, shared_ptr<BPLinkedList> > new_directory;
    for (const auto &pair : this->directory)
    {
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        new_directory[prefix + "0"] = linkedlist;
        new_directory[prefix + "1"] = linkedlist;

        if (linkedlist->size > 1)
        {
            vector<string> shared_keys = this->get_keys_sharing_linkedlist(new_directory, prefix + "0"); // Shared keys should only be prefix + "0" and prefix + "1"
            this->rehash_linked_list(&new_directory, prefix + "0", shared_keys);
        }
    }
    this->directory = new_directory;
    }

void BPDirectory::evict_page(PageFrame* pageFrame) {
    // remove the pageFrame from the directory hash map and linked list
    string source = pageFrame->sst_name + to_string(pageFrame->page_number);
    string directory_key = this->hash_string(source);
    this->directory[directory_key]->remove_page_frame(pageFrame);

    // free the memory when you're done
    for (int i = 0; i < pageFrame->num_pairs_in_page; ++i)
    {
        pageFrame->page_content[i].~pair<db_key_t, db_val_t>();
    }
    free(pageFrame);
}

void BPDirectory::edit_directory_size(int new_maximum_bp_size)
{
    while (this->current_bp_size > new_maximum_bp_size)
    {   
        // TODO evict items until it fits
        // this->evict_directory();
        // this->current_bp_size -= whatever got evicted
    }
    // TODO AMY: figure out how to shrink the directory table keys
    // might not do this lol

    this->maximum_bp_size = new_maximum_bp_size;
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
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        (*directory)[prefix] = linkedlist;
    }
}

vector<string> BPDirectory::get_keys_sharing_linkedlist(map<string, shared_ptr<BPLinkedList> > directory, string key)
{
    vector<string> shared_keys;
    shared_ptr<BPLinkedList> shared_linkedlist = directory[key];

    for (const auto &pair : directory)
    {
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;

        if (linkedlist == shared_linkedlist)
        {
            shared_keys.push_back(prefix);
        }
    }

    return shared_keys;
}

void BPDirectory::print_directory() {
    cout << "The number of pages in the directory: " << this->current_num_items << endl;
    for (auto prefix = this->directory.begin(); prefix != this->directory.end(); ++prefix)
    {
        std::cout << "The directory prefix: " << prefix->first << std::endl;
        prefix->second->print_list();
    }
}

void BPDirectory::free_all_pages()
{
    for (auto prefix = this->directory.begin(); prefix != this->directory.end(); ++prefix)
    {
        prefix->second->free_all_pages();
    }
}



#include "bp_directory.h"
#include "xxhash.h"
#include "LRUCache.h"
#include "bp_pageframe.h"
#include "ClockBitmap.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include <cstring>
#include <cmath>

#define PAGE_SIZE 4096

using namespace std;

BPDirectory::BPDirectory(string eviction_policy, int initial_num_bits, int maximum_bp_size, int maximum_num_items_threshold)
{
    this->policy = eviction_policy; // This is the eviction polivy, should be "LRU" or "clock"

    this->initial_num_bits = initial_num_bits; // This is the initial number of prefix bits in the directory
    this->current_num_bits = initial_num_bits; // This tracks the current number of prefix bits in the directory

    this->current_bp_size = 0;               // This is the total number of bytes of items currently in the buffer pool
    this->maximum_bp_size = maximum_bp_size; // This is the max number of bytes that the buffer pool can contain before eviction policy should kick in

    this->current_num_items = 0;                                     // This is the total number of items currently in the buffer pool
    this->maximum_num_items_threshold = maximum_num_items_threshold; // This is the maximum number of items we can have in the buffer pool before expansion, which results in doubling
    this->page_id = 0;                                               // The total number of pages we've interacted before (placeholder value)

    for (const string &prefix : generate_binary_strings(this->initial_num_bits))
    {
        this->directory[prefix] = make_shared<BPLinkedList>();
    }

    this->lru_cache = make_shared<LRUCache>();
    this->update_directory_keys();
    this->clock_hand_key = 0;
    this->clock_hand_location = this->directory[this->directory_keys[0]]->head;
    this->clock_cycle_count = 1;
}

// Clock eviction policy(2)
shared_ptr<PageFrame> BPDirectory::clock_find_victim()
{
    // find the victim using clock eviction policy and return the victim
    // We will start at the clock hand and go around the clock until we find a victim
    // If we don't find a victim, we will return nullptr
    // If we find a victim, we will return the victim and update the clock hand
    // We will also update the clock hand to point to the next item in the clock bitmap
    // We will also update the clock hand key and clock hand index to point to the next item in the clock bitmap

    while (true)
    {
        this->move_clock_hand();

        shared_ptr<PageFrame> potential_victim = this->clock_hand_location;
        if (potential_victim == nullptr)
        {
            continue;
        }

        if (this->directory[this->directory_keys[this->clock_hand_key]]->clock_cycle_count < this->clock_cycle_count)
        {
            if (potential_victim->get_reference_bit() == 0)
            {
                return potential_victim;
            }
            else
            {
                potential_victim->set_reference_bit(0);
            }
            if (this->clock_hand_location->next == nullptr)
            {
                this->directory[this->directory_keys[this->clock_hand_key]]->clock_cycle_count = this->clock_cycle_count;
            }
        }
    }
}

void BPDirectory::move_clock_hand()
{
    // update the reference bit of the victim to 1
    // update the clock hand to point to the next item in the clock bitmap
    if (this->clock_hand_location == nullptr)
    {
        this->clock_hand_key++;
        if ((size_t)this->clock_hand_key >= this->directory_keys.size())
        {
            this->clock_hand_key = 0;
            this->clock_cycle_count++;
        }
        this->clock_hand_location = this->directory[this->directory_keys[this->clock_hand_key]]->head;
    }
    else
    {
        this->clock_hand_location = this->clock_hand_location->next;
    }
}

void BPDirectory::update_directory_keys()
{
    this->directory_keys.clear();
    for (auto it = this->directory.begin(); it != this->directory.end(); ++it)
    {
        this->directory_keys.push_back(it->first);
    }
    this->clock_hand_key = 0;
    this->clock_hand_location = this->directory[this->directory_keys[0]]->head;
}

void BPDirectory::set_maximum_bp_size(int value)
{
    this->maximum_bp_size = value;
    evict_until_under_max_bp_size();
}

void BPDirectory::insert_page(void *page_content, int num_pairs_in_page, string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    this->current_bp_size += PAGE_SIZE;
    void *malloc_page;
    malloc_page = malloc(PAGE_SIZE);
    memcpy(malloc_page, page_content, PAGE_SIZE);

    shared_ptr<PageFrame> newNode = this->directory[directory_key]->add_page_frame(malloc_page, num_pairs_in_page,
                                                                                   sst_name, page_number);
    this->current_num_items += 1;
    this->page_id += 1;
    newNode->set_id(this->page_id);

    if (this->policy == "LRU")
    {
        this->lru_cache->put(newNode->page_number, newNode);
    }
    else
    { // clock
      // don't need to do anything for now. We will update the clock hand when we need to evict
    }

    this->evict_until_under_max_bp_size();

    if (this->current_num_items >= this->maximum_num_items_threshold)
    {
        // This should rehash all the linkedlists greater than length 1 after expansion
        this->extend_directory();
        return;
    }
    else
    {
    }

    // Rehash as soon as chain length gets to be greater than 1
    // Retrieved from: https://piazza.com/class/lckjb9lrfaa57i/post/78
    // Case 1: a bucket pointed at by just one pointer from the directory. This bucket already expanded since the last time the directory expanded. We need to patiently wait until the directory expands again before we can expand this bucket.
    // Case 2: a bucket pointed at by more than one pointer from the directory. We should expand this bucket immediately when it overflows.
    // This is Case 2
    vector<string> shared_keys = this->get_keys_sharing_linkedlist(this->directory, directory_key);
    if (this->directory[directory_key]->size > 1 && shared_keys.size() > 1)
    {
        this->rehash_linked_list(&this->directory, directory_key, shared_keys);
    }
}

// Shrink API (2)
void BPDirectory::evict_until_under_max_bp_size()
{
    shared_ptr<PageFrame> pageToEvict = nullptr;
    while (this->current_bp_size > this->maximum_bp_size)
    {
        if (this->policy == "LRU")
        {
            // LRU eviction policy (2)
            pageToEvict = this->lru_cache->evict_one_page_item();
        }
        else
        { // clock
            pageToEvict = this->clock_find_victim();
        }
        if (pageToEvict != nullptr)
        {
            this->current_num_items--;
            this->current_bp_size -= PAGE_SIZE;
            this->evict_page(pageToEvict);
        }
    }
}

void BPDirectory::mark_item_as_used(shared_ptr<PageFrame> pageFrame)
{
    if (this->policy == "LRU")
    {
        this->lru_cache->mark_item_as_used(pageFrame);
    }
    else
    { // clock
        pageFrame->set_reference_bit(1);
    }
}

shared_ptr<PageFrame> BPDirectory::get_page(string sst_name, int page_number)
{
    string source = sst_name + to_string(page_number);
    string directory_key = this->hash_string(source);

    shared_ptr<PageFrame> pageFrame = this->directory[directory_key]->find_page_frame(sst_name, page_number);
    this->mark_item_as_used(pageFrame);
    return pageFrame;
}

// Extendible hash buffer pool (1)
void BPDirectory::extend_directory()
{
    this->current_num_bits++;
    this->maximum_num_items_threshold *= 2;

    // Double the size of the directory and make it point to the same linkedlist
    map<string, shared_ptr<BPLinkedList>> new_directory;
    for (const auto &pair : this->directory)
    {
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        new_directory[prefix + "0"] = linkedlist;
        new_directory[prefix + "1"] = linkedlist;
    }

    for (const auto &pair : this->directory)
    {
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        if (linkedlist->size > 1)
        {
            vector<string> shared_keys = this->get_keys_sharing_linkedlist(new_directory, prefix + "0"); // Shared keys should only be prefix + "0" and prefix + "1"
            this->rehash_linked_list(&new_directory, prefix + "0", shared_keys);
        }
    }
    this->directory = new_directory;
    this->update_directory_keys();
}

void BPDirectory::evict_page(shared_ptr<PageFrame> pageFrame)
{
    // remove the pageFrame from the directory hash map and linked list
    string source = pageFrame->sst_name + to_string(pageFrame->page_number);
    string directory_key = this->hash_string(source);
    this->directory[directory_key]->remove_page_frame(pageFrame);
}

void BPDirectory::evict_pages_associated_with_files(size_t memtable_size, vector<string> filenames)
{
    for (const auto &filename : filenames)
    {
        ifstream file(filename, std::ios::binary);
        if (filesystem::exists(filename))
        {
            file.seekg(0, std::ios::end);
            int file_size = static_cast<int>(file.tellg());
            int num_pages_in_file = file_size / PAGE_SIZE;
            for (int page_number = 0; page_number < num_pages_in_file + 1; page_number++)
            {
                string source = filename + to_string(page_number);
                string directory_key = this->hash_string(source);
                try
                {
                    shared_ptr<PageFrame> pageFrame = this->directory[directory_key]->find_page_frame(filename,
                                                                                                      page_number);
                    this->directory[directory_key]->remove_page_frame(pageFrame);
                }
                catch (out_of_range &e)
                {
                    continue;
                }
            }
        }
        else
        {
            continue;
        }
    }
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

void BPDirectory::set_policy(string policy)
{
    if (policy.compare("LRU") != 0 || policy.compare("clock") != 0)
    {
        throw invalid_argument("This is not a valid policy. Please pass in LRU or clock.");
    }
    else
    {
        this->policy = policy;
    }
}

void BPDirectory::rehash_linked_list(map<string, shared_ptr<BPLinkedList>> *directory, string key, vector<string> shared_keys)
{
    // Right now we have a situation where multiple entries both point to the same linkedlist
    // but this linkedlist need to be rehashed between the multiple entries because the length is too long
    map<string, shared_ptr<BPLinkedList>> key_to_new_linkedlist;

    for (const auto &shared_key : shared_keys)
    {
        key_to_new_linkedlist[shared_key] = make_shared<BPLinkedList>();
    }

    shared_ptr<PageFrame> current = (*directory)[key]->head;
    while (current != nullptr)
    {
        string source = current->sst_name + to_string(current->page_number);
        string directory_key = this->hash_string(source);
        if (key_to_new_linkedlist.find(directory_key) == key_to_new_linkedlist.end())
        {
            (*directory)[directory_key]->add_page_frame(current->page_content, current->num_pairs_in_page,
                                                        current->sst_name, current->page_number);
        }
        else
        {
            key_to_new_linkedlist[directory_key]->add_page_frame(current->page_content, current->num_pairs_in_page,
                                                                 current->sst_name, current->page_number);
        }
        current = current->next;
    }

    for (const auto &pair : key_to_new_linkedlist)
    {
        string prefix = pair.first;
        shared_ptr<BPLinkedList> linkedlist = pair.second;
        (*directory)[prefix] = linkedlist;
    }
}

vector<string> BPDirectory::get_keys_sharing_linkedlist(map<string, shared_ptr<BPLinkedList>> directory, string key)
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

void BPDirectory::print_directory()
{
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
        int num_pages_freed = prefix->second->free_all_pages();
        this->current_bp_size -= num_pages_freed * PAGE_SIZE;
    }
}

void BPDirectory::insert_bloom_filter(shared_ptr<BloomFilter> bf)
{
    string sst_name = bf->sst_name;
    // put *bf into bloom_filters with key sst_name
    this->bloom_filters[sst_name] = bf;
    this->evict_until_under_max_bp_size();
}

void BPDirectory::remove_bloom_filter(string sst_name)
{
    this->current_bp_size -= this->bloom_filters[sst_name]->size;
    this->bloom_filters.erase(sst_name);
}

void BPDirectory::load_bloom_filter(string sst_name)
{
}

shared_ptr<BloomFilter> BPDirectory::get_bloom_filter(string sst_name)
{
    // print this->bloom_filters

    // print out the sst_name of all the bloom filters
    //    for (auto it = this->bloom_filters.begin(); it != this->bloom_filters.end(); ++it) {
    //        cout << "The sst_name of the bloom filter: " << it->first << endl;
    //    }
    return this->bloom_filters[sst_name];
}

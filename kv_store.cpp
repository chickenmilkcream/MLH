#include "bp_directory.h"
#include "memtable.h"
#include "kv_store.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;

KeyValueStore::KeyValueStore(int memtable_size, string eviction_policy, int initial_num_bits, int maximum_bp_size, int maximum_num_items_threshold)
{
    this->memtable = Memtable(memtable_size);
    this->buffer_pool = BPDirectory(eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold);

    this->sst_num = 1;
    this->memtable_size = memtable_size;
    this->page_size = 4096;
    this->get_method = "binary";
}

void KeyValueStore::open_db(string db)
{
    // TODO TEAM: clarify load all SSTs??
    // TODO TEAM: discuss with team what else to do here
}

void KeyValueStore::close_db()
{
    this->serialize();
    this->buffer_pool.free_all_pages();
}

db_val_t KeyValueStore::get(db_key_t key)
{
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));

    try
    {
        // Try to look in the memtable for the key
        db_val_t memtable_result = this->memtable.get(key);
        return memtable_result;
    }
    catch (std::invalid_argument &e)
    {
        // Couldn't find it in the memtable
        // Loop through the SSTs from latest to oldest
        for (int i = this->sst_num - 1; i > 0; i--)
        {
            // Open the file
            std::string s = "sst_" + to_string(i) + ".bin";
            int file = open(s.c_str(), O_RDONLY);
            if (file == -1) continue; // If we can't find open the file

            int position;
            int page;
            if (this->get_method.compare("binary") == 0)
            {
                page = binary_search_page_containing_target(s, file, key);
                if (page == -1) continue;
                position = binary_search_target_within_page(file, page, key);
            }
            else if (this->get_method.compare("btree") == 0)
            {
                cout << "TODO JUN" << endl;
            }

            if (position == -1) continue; // If we can't find min_key in the file

            // Jump to the position in the file with the key
            off_t offset = position * pairSize;
            pair<db_key_t, db_val_t> kv_pair;
            ssize_t bytes_read = pread(file, &kv_pair, pairSize, offset);
            return kv_pair.second;
        }

        // After going through all the SSTs, we still can't find it
        throw invalid_argument("NOT FOUND");
    }
}

void KeyValueStore::put(db_key_t key, db_val_t val)
{
    this->memtable.put(key, val);

    if (this->memtable.size == this->memtable.max_size)
    {
        this->serialize();
        this->sst_num += 1;
    }
}

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key)
{
    vector<pair<db_key_t, db_val_t> > memtable_results = this->memtable.scan(min_key, max_key);
    vector<pair<db_key_t, db_val_t> > sst_results;
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));

    // Loop through the SSTs from latest to oldest
    for (int i = this->sst_num - 1; i > 0; i--)
    {
        // Open the file
        std::string s = "sst_" + to_string(i) + ".bin";
        int file = open(s.c_str(), O_RDONLY);
        if (file == -1) continue; // If we can't find open the file

        int position;
        if (this->get_method.compare("binary") == 0)
        {
            position = binary_search_smallest(file, min_key);
        }
        else if (this->get_method.compare("btree") == 0)
        {
            cout << "todo JUN" << endl;
        }

        if (position == -1)
            continue; // If we can't find min_key in the file

        // Jump to the position in the file with the min_key
        off_t offset = position * pairSize;

        // Continuously scan until we reach max_key or end of file
        while (true)
        {
            pair<db_key_t, db_val_t> kv_pair;
            ssize_t bytes_read = pread(file, &kv_pair, pairSize, offset);
            if (kv_pair.first > max_key || bytes_read == 0) break;
            sst_results.push_back(kv_pair);
            offset += pairSize;
        }
        close(file);
    }

    // Return the memtable and SST results
    memtable_results.insert(memtable_results.end(), sst_results.begin(), sst_results.end());
    return memtable_results;
}

void KeyValueStore::print() { this->memtable.print(); }

void KeyValueStore::write_to_file(vector<pair<db_key_t, db_val_t> > vector_mt)
{
    std::string s = "sst_" + to_string(this->sst_num) + ".bin";
    const char *filename = s.c_str();

    // write the vector to a binary file
    int outFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(outFile, &vector_mt[0], vector_mt.size() * sizeof(pair<db_key_t, db_val_t>));
    close(outFile);
}

void KeyValueStore::read_from_file(const char *filename)
{
    // read the vector from the binary file
    vector<pair<db_key_t, db_val_t> > pairs;
    int inFile = open(filename, O_RDONLY);
    off_t offset = 0;

    while (true) {
        pair<db_key_t, db_val_t> kv_pair;
        ssize_t bytes_read = pread(inFile, &kv_pair, sizeof(pair<db_key_t, db_val_t>), offset);
        if (bytes_read == 0) break;
        pairs.push_back(kv_pair);
        offset += sizeof(pair<db_key_t, db_val_t>);
    }
    close(inFile);

    // print the read vector
    for (auto p : pairs)
    {
        cout << p.first << " " << p.second << endl;
    }
}

void KeyValueStore::serialize()
{
    // TODO BUG: if we set min_key as a negative number, scan doesn't work and return 0 apirs. This might have something to do with key as uint64 being converted to unsigned int somehow.
    vector<pair<db_key_t, db_val_t> > vector_mt = this->memtable.scan(0, 999999999);

    this->write_to_file(vector_mt);

    this->memtable = Memtable(this->memtable_size);
}

int KeyValueStore::binary_search_smallest(int file, db_key_t target)
{
    // This function finds the position of the smallest key that is greater than target

    // Compute the number of key-value pairs in the file
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int numPairs = lseek(file, 0, SEEK_END) / pairSize;

    // Binary search loop
    int low = 0, high = numPairs - 1, result = numPairs;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        pair<db_key_t, db_val_t> pair;
        pread(file, &pair, pairSize, mid * pairSize);

        if (pair.first >= target)
        {
            result = mid;
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return result;
}

int KeyValueStore::binary_search_page_containing_target(string sst_name, int file, db_key_t target)
{
    // This function finds the page that contains the target

    // Compute the number of key-value pairs in the file
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int num_pairs_in_page = this->page_size / pairSize;
    int numPages = lseek(file, 0, SEEK_END) / (this->page_size);

    // Binary search loop
    int low = 0, high = numPages - 1, result = numPages;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        pair<db_key_t, db_val_t> pairs[num_pairs_in_page];

        // Check in the buffer for the page
        try
        {
            pair<db_key_t, db_val_t> *result = this->buffer_pool.get_page(sst_name, mid)->page_content;
            memcpy(pairs, result, num_pairs_in_page * sizeof(pair<db_key_t, db_val_t>));
        }
        catch (std::out_of_range &e)
        {
            // Couldn't find it in the buffer pool
            // Store the page and return the value
            pread(file, pairs, this->page_size, mid * this->page_size);
            this->buffer_pool.insert_page(pairs, num_pairs_in_page, sst_name, mid);
        }

        if (target >= pairs[0].first && target <= pairs[num_pairs_in_page - 1].first)
        {
            return mid;
        }
        else if (pairs[0].first > target)
        {
            result = mid;
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return -1;
}

int KeyValueStore::binary_search_target_within_page(int file, int page, db_key_t target)
{
    // This function finds the position of the key that is equal to target

    // Compute the number of key-value pairs in the file
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int numPairs = this->page_size / pairSize;

    // Binary search loop
    int low = page * numPairs, high = low + numPairs - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;

        pair<db_key_t, db_val_t> pair;
        pread(file, &pair, pairSize, mid * pairSize);

        if (pair.first == target)
        {
            return mid;
        }
        else if (pair.first < target)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return -1;
}

void KeyValueStore::set_get_method(string get_method)
{
    if (get_method.compare("binary") != 0 || get_method.compare("btree") != 0)
    {
        throw invalid_argument("This is not a valid get method. Please pass in binary or btree.");
    }
    else
    {
        this->get_method = get_method;
    }
}

void KeyValueStore::set_page_size(int page_size)
{
    if (page_size % 16 != 0)
    {
        throw invalid_argument("Please make sure that the page size you pass in is a factor of 16.");
    }
    else if (this->memtable_size % page_size != 0)
    {
        throw invalid_argument("Please make sure that the SST (memtable_size) will fit an integer number of pages.");
    }
    else
    {
        this->page_size = page_size;
    }
}
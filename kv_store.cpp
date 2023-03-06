#include "kv_store.h"
#include "memtable.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

KeyValueStore::KeyValueStore(uint64_t memtable_size)
{
    this->memtable = Memtable(memtable_size);
    this->num_sst = 1;
    this->memtable_size = memtable_size;
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
    // TODO TEAM: discuss with team what else to do here
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
        for (int i = this->num_sst - 1; i > 0; i--)
        {
            // Open the file
            std::string s = "sst_" + to_string(i) + ".bin";
            int file = open(s.c_str(), O_RDONLY);
            if (file == -1) continue; // If we can't find open the file
            
            // TODO AMY: implement the logic for 
            // if page in BP: return it 
            // otherwise: add it to the BP after getting it
            // Also the same thing in the binary search functions

            // TODO AMY: change binary functions so that it looks for 4kb pages at a time instead of individual key-value pairs

            int position;
            if (this->get_method.compare("binary") == 0)
            {
                position = binary_search_exact(file, key);
            }
            else if (this->get_method.compare("btree") == 0)
            {
                cout << "TODO JUN" << endl;
            }

            if (position == -1)
                continue; // If we can't find min_key in the file

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
        this->num_sst += 1;
    }
}

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key)
{
    vector<pair<db_key_t, db_val_t> > memtable_results = this->memtable.scan(min_key, max_key);
    vector<pair<db_key_t, db_val_t> > sst_results;
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));

    // Loop through the SSTs from latest to oldest
    for (int i = this->num_sst - 1; i > 0; i--)
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
    std::string s = "sst_" + to_string(this->num_sst) + ".bin";
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

int KeyValueStore::binary_search_exact(int file, db_key_t target)
{
    // This function finds the position of the key that is equal to target

    // Compute the number of key-value pairs in the file
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int numPairs = lseek(file, 0, SEEK_END) / pairSize;

    // Binary search loop
    int low = 0, high = numPairs - 1;
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
        cout << "This is not a valid get method. Please pass in binary or btree." << endl;
    }
    else
    {
        this->get_method = get_method;
    }
}
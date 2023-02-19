
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
}

void KeyValueStore::kv_open(string db)
{
    // todo load all SSTs i think...
}

void KeyValueStore::kv_close()
{
}

db_val_t KeyValueStore::get(db_key_t key)
{
    try
    {
        return this->memtable.get(key);
    }
    catch (std::invalid_argument &e)
    {
        throw invalid_argument("AHH");
    }
    // todo check SSTs
    // return this->memtable.get(key);
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
    return this->memtable.scan(min_key, max_key);
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
    vector<pair<db_key_t, db_val_t> > pairs2;

    int inFile = open(filename, O_RDONLY);

    off_t offset = 0;
    pairs2.resize(this->memtable_size/16);
    for (int i = 0; i < this->memtable_size / 16; i++)
    {
        pread(inFile, &pairs2[i], sizeof(pair<db_key_t, db_val_t>), offset);
        offset += sizeof(pair<db_key_t, db_val_t>);
    }
    close(inFile);

    // print the read vector
    for (auto p : pairs2)
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

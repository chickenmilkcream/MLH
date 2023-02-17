
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
        cerr << e.what() << endl;
    }
    // todo check SSTs
}

void KeyValueStore::put(db_key_t key, db_val_t val)
{
    this->memtable.size ++;
    this->memtable.put(key, val);
    // todo talk to json about increasing capacity and then calling serialize
    if (this->memtable.size == this->memtable.max_size) { 
        this->serialize();
        this->num_sst += 1;
    }
}

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key)
{
    return this->memtable.scan(min_key, max_key);
}

void KeyValueStore::write_to_file(vector<pair<db_key_t, db_val_t> > vector_mt){
    std::string s = "sst_" + to_string(this->num_sst) + ".bin";
    const char * filename = s.c_str();
    // std::ofstream outputFile(filename, std::ios::binary);
    // for (const auto &p : vector_mt)
    // {
    //     outputFile.write((char *)&p, sizeof(p));
    // }

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC);
    ssize_t nwritten = pwrite(fd, &vector_mt, sizeof(vector_mt), -1);
    if (nwritten != sizeof(vector_mt))
    {
        cerr << "Error writing to file." << endl;
        return;
    }

    // for (auto &p : vector_mt)
    // {
    //     ssize_t nwritten = pwrite(fd, &p, sizeof(p), -1);
    //     if (nwritten != sizeof(p)) {
    //         cerr << "Error writing to file." << endl;
    //         return;
    //     }
    // }

    close(fd);
}

void KeyValueStore::serialize()
{
    vector<pair<db_key_t, db_val_t> > vector_mt = this->memtable.scan((uint64_t)-999999999, (uint64_t)999999999);
    this->write_to_file(vector_mt);
    // TODO: free memtable 
    // delete this->memtable;
    this->memtable = Memtable(this->memtable_size);


    //    vector<std::pair<int, int>> vec;
    //    ifstream inputFile("sst_1.bin", ios::binary);
    //
    //    while (inputFile.good()) {
    //        pair < db_key_t, db_val_t > p;
    //        inputFile.read((char *) &p, sizeof(p));
    //        vec.emplace_back(p);
    //    }
    //    inputFile.close();
    //
    //    for (const auto &p: vec) {
    //        cout << p.first << " " << p.second << endl;
    //    }
    //
    //    return 0;
}

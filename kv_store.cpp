
#include "kv_store.h"
#include "memtable.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
void KeyValueStore::open(string db)
{
}

void KeyValueStore::close()
{
}

uint64_t KeyValueStore::get(uint64_t key)
{
    return 0;
}

void KeyValueStore::put(uint64_t key, uint64_t val)
{
}

void KeyValueStore::scan(uint64_t min_key, uint64_t max_key)
{
}

void KeyValueStore::serialize()
{
    vector<pair<db_key_t, db_val_t>> vector_mt = this->memtable.scan(-999999999, 999999999);
    string filename = "sst_" + to_string(num_sst) + ".bin";
    std::ofstream outputFile("sst_1.bin", std::ios::binary);
    for (const auto &p: vector_mt) {
        outputFile.write((char *) &p, sizeof(p));
    }
    outputFile.close();
}


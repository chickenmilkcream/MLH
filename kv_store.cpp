
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

//    vector<pair<db_key_t, db_val_t>> mt_vector = mt.scan(0, 999);
//    for (pair<db_key_t, db_val_t> pair: mt_vector) {
//        cout << pair.first << ", " << pair.second << endl;
//    }
//    cout << "size: " << mt_vector.size() << endl;
//
//
//    std::ofstream outputFile("sst_1.bin", std::ios::binary);
//    for (const auto &p: mt_vector) {
//        outputFile.write((char *) &p, sizeof(p));
//    }
//    outputFile.close();
//
//
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


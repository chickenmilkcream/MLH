#ifndef KV_STORE_H
#define KV_STORE_H

#include <string>
#include <cstdint>
#include <memtable.h>

using namespace std;

class KeyValueStore
{
    public:        
        KeyValueStore(uint32_t memtable_size);
        void open(string db);
        void close();
        uint64_t get(uint64_t key);
        void put(uint64_t key, uint64_t val);
        void scan(uint64_t min_key, uint64_t max_key); // not sure about return

    private:
        Memtable memtable;
        void serialize();
};

#endif

#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <vector>
#include <functional>
#include <cstdint>
#include <string>
#include "avl_node.h"

class BloomFilter {
public:
    BloomFilter(uint32_t num_bits_per_entry, uint32_t num_hash_functions, const char *filename,
                uint32_t num_entries);

    void insert(db_val_t value);
    bool contains(db_val_t value) const;
    void write_to_file(const char *filename);
    void read_from_file(const char *filename);
    std::vector<int> bit_array;
    size_t size;
private:

    uint32_t num_bits_per_entry;
    uint32_t num_hash_functions;
    uint32_t num_bits;

    uint32_t hash(db_val_t value, uint32_t i) const;

    string sst_name;

    const char *file_name;
};

#endif // BLOOMFILTER_H
#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <vector>
#include <functional>
#include <cstdint>
#include <string>
#include "avl_node.h"

# define NUM_BITS_PER_ENTRY 5

class BloomFilter {
public:

    std::vector<int> bit_array;
    size_t size;
    string sst_name;
    uint32_t num_bits;
    uint32_t num_bits_per_entry{};
    uint32_t num_hash_functions;

    BloomFilter(string filename, uint32_t num_entries, uint32_t num_hash_functions);

    void insert(db_val_t value);
    bool contains(db_val_t value) const;
    void write_to_file(string filename);
    void set_parameters(uint32_t num_bits, uint32_t num_hash_functions);
    void set_sst_name(string filename);
    void read_from_file(string filename);

private:
    uint32_t hash(db_val_t value, uint32_t i) const;
};

#endif // BLOOMFILTER_H
#include "BloomFilter.h"
#include "avl_node.h"
#include <cmath>
#include <functional>
#include <cstring>
#include <string>


BloomFilter::BloomFilter(uint32_t num_bits_per_entry, uint32_t num_hash_functions, uint32_t num_entries) {
    this->num_hash_functions = num_hash_functions;
    this->bit_array = std::vector<bool>(num_bits_per_entry * num_entries);

}

void BloomFilter::insert(db_val_t value) {
    for (uint32_t i = 0; i < num_hash_functions; ++i) {
        uint32_t index = hash(value, i) % num_bits;
        bit_array[index] = true;
    }
}

bool BloomFilter::contains(db_val_t value) const {
    for (uint32_t i = 0; i < num_hash_functions; ++i) {
        uint32_t index = hash(value, i) % num_bits;
        if (!bit_array[index]) {
            return false;
        }
    }
    return true;
}

uint32_t BloomFilter::hash(db_val_t value, uint32_t i) const {
    std::hash<db_val_t> hasher;
    uint64_t hash_seed = static_cast<uint64_t>(hasher(value) + i);
    return static_cast<uint32_t>((hash_seed * 0x9e3779b1) % 0xFFFFFFFF);
}



#include <iostream>

int main() {
    BloomFilter filter(5, 7, 3);

//    filter.insert(1);
//    filter.insert(2);

    std::cout << "contains 1: " << filter.contains(1) << std::endl;
    std::cout << "contains 2: " << filter.contains(2) << std::endl;
    std::cout << "contains 0: " << filter.contains(0) << std::endl;

    return 0;
}
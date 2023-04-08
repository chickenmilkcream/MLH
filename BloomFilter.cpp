#include "BloomFilter.h"
#include "avl_node.h"
#include <cmath>
#include <functional>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>


BloomFilter::BloomFilter(uint32_t num_bits_per_entry, uint32_t num_hash_functions, const char *filename,
                         uint32_t num_entries)
        : num_bits_per_entry(num_bits_per_entry),
          num_hash_functions(num_hash_functions),
          num_bits(num_bits_per_entry * num_entries) {
    bit_array.resize(num_bits, false);
    this->file_name = filename;
    this->size = num_bits_per_entry * num_entries * sizeof(db_key_t);
}

void BloomFilter::insert(db_val_t value) {
    for (uint32_t i = 0; i < num_hash_functions; ++i) {
        uint32_t index = hash(value, i) % num_bits;
//        std::cout << "index: " << index << std::endl;
        bit_array[index] = 1;
    }
}

bool BloomFilter::contains(db_val_t value) const {
    for (uint32_t i = 0; i < num_hash_functions; ++i) {
        uint32_t index = hash(value, i) % num_bits;
        if (0 == bit_array[index]) {
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

void BloomFilter::write_to_file(const char *filename) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<char *>(&bit_array[0]), num_bits);
    file.close();
}

void BloomFilter::read_from_file(const char *filename) {
    // read contents from binary file into vector<int> until end of file but we don't know how many bits
    // are in the file so we need to read until the end of the file
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    bit_array.resize(file_size);
    file.read(reinterpret_cast<char *>(&bit_array[0]), file_size);
    file.close();
}

//
//int main() {
//    BloomFilter filter(10000, 7, nullptr, 2);
//
//    filter.insert(1);
//    filter.insert(2);
//
//    std::cout << "contains 1: " << filter.contains(1) << std::endl;
//    std::cout << "contains 2: " << filter.contains(2) << std::endl;
//    std::cout << "contains 0: " << filter.contains(0) << std::endl;
//
//    // print filter.bit_array
//    for (int i = 0; i < filter.bit_array.size(); ++i) {
//        std::cout << filter.bit_array[i] << " ";
//    }
//
//    // make a new copy of filter.bit_array called oof without alias
//    std::vector<int> oof(filter.bit_array);
//
//    std::cout << std::endl;
//
//    filter.write_to_file("test.bin");
//    filter.read_from_file("test.bin");
//
//    // print filter.bit_array
//    for (int i = 0; i < filter.bit_array.size(); ++i) {
//        std::cout << filter.bit_array[i] << " ";
//    }
//
//    std::vector<int> rip(filter.bit_array);
//
//    // assert that oof and rip are the same
//    for (int i = 0; i < oof.size(); ++i) {
//        assert(oof[i] == rip[i]);
//    }
//    assert(oof == rip);
//
//    // print out integers from the binary file
//    std::cout << "contains 1: " << filter.contains(1) << std::endl;
//    std::cout << "contains 2: " << filter.contains(2) << std::endl;
//    std::cout << "contains 0: " << filter.contains(0) << std::endl;
//    // print out the first 100 integers
//
//    return 0;
//}
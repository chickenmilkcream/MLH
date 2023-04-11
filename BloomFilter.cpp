#include "BloomFilter.h"
#include "avl_node.h"
#include <cmath>
#include <functional>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>


BloomFilter::BloomFilter(string filename, uint32_t num_entries, uint32_t num_hash_functions)
        : num_hash_functions(num_hash_functions),
          num_bits(NUM_BITS_PER_ENTRY * num_entries) {
    bit_array.resize(num_bits, false);
    this->sst_name = filename;
    this->size = NUM_BITS_PER_ENTRY * num_entries * sizeof(db_key_t);
}

void BloomFilter::insert(db_val_t value) {
    for (uint32_t i = 0; i < num_hash_functions; ++i) {
        uint32_t index = hash(value, i) % num_bits;
        bit_array[index] = true;
    }
}

bool BloomFilter::contains(db_val_t value) const {
    // cout << "checking if value " << value << " is in bloom filter" << endl;
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

void BloomFilter::write_to_file(string filename) {
    std::ofstream outfile(filename, std::ios::binary);
    // Write vector data to file
    outfile.write(
            reinterpret_cast<const char*>(
                    *reinterpret_cast<std::uint64_t* const*>(&bit_array)),
            bit_array.size() * sizeof(bool)
    );
    // Close file
    outfile.close();
}

void BloomFilter::read_from_file(string filename) {
    // read the file back
    std::ifstream infile(filename, std::ios::binary);
    // Read vector data from file
    infile.read(
            reinterpret_cast<char*>(
                    *reinterpret_cast<std::uint64_t**>(&bit_array)),
            bit_array.size() * sizeof(bool)
    );
// Close file
    infile.close();
}

void BloomFilter::set_sst_name(string filename) {
    this->sst_name = std::move(filename);
}

void BloomFilter::set_parameters(uint32_t num_entries, uint32_t num_hash_functions) {
    this->num_bits = num_entries * NUM_BITS_PER_ENTRY;
    this->num_hash_functions = num_hash_functions;
    bit_array.resize(this->num_bits, false);
}


//int main() {
//    BloomFilter filter(10000, 7, "gg", 2);
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
//    std::cout << rip.size() << std::endl;
//
//    // print out integers from the binary file
//    std::cout << "contains 1: " << filter.contains(1) << std::endl;
//    std::cout << "contains 2: " << filter.contains(2) << std::endl;
//    std::cout << "contains 0: " << filter.contains(0) << std::endl;
//    // print out the first 100 integers
//
//    return 0;
//}

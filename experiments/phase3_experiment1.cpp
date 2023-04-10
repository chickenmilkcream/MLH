#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "../kv_store.h"
#include <random>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <unordered_map>
#include <vector>


int main() {
    std::vector<double> put_runtimes;
    std::vector<double> get_runtimes;
    std::vector<double> scan_runtimes;

    size_t memtable_size = 1048576;                        // This is 1 MB
    string policy = "clock";
    int initial_num_bits = 2;
    int max_bp_size = 10485760;                            // This is 10 MB
    int maximum_num_items_threshold = 4;
    int num_kv_pairs_gigabyte = 1073741824 / DB_PAIR_SIZE; // This is 1 GB
    int chunk_size = 104857600/ DB_PAIR_SIZE;              // This is 100 MB

    std::mt19937 gen(69);

    KeyValueStore db = KeyValueStore(memtable_size, policy, initial_num_bits, max_bp_size, maximum_num_items_threshold);
    // Start the overall timer
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_kv_pairs_gigabyte; i += chunk_size) {
        // Start the chunk timer
        auto chunk_start = std::chrono::high_resolution_clock::now();

        // Insert chunk of data into database
        
        for (int j = 0; j < chunk_size; j++) {
            db_key_t key = i + j;
            db_val_t val = rand();
            db.put(key, val);
        }

        // Stop the chunk timer
        auto chunk_end = std::chrono::high_resolution_clock::now();

        // Calculate the time it took to insert the chunk
        auto chunk_time = std::chrono::duration_cast<std::chrono::milliseconds>(chunk_end - chunk_start).count();

        // Store the chunk time in the vector
        put_runtimes.push_back(chunk_time);

        // Output progress message
        std::cout << "Inserted " << (i) << " key-value pairs so far (" << ((i) * 100 / num_kv_pairs_gigabyte) << "%)" << chunk_time << std::endl;
    }

    // Stop the overall timer
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the total time it took to insert all the data
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Output the total time
    std::cout << "Inserted " << num_kv_pairs_gigabyte << " key-value pairs in " << total_time << " ms" << std::endl;

    return 0;
}

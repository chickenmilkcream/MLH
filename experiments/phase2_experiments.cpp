#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "../kv_store.h"
#include <random>

int main() {
    int arr[] = {
        2048,
        4096,
        6144,
        8192
    };
    std::vector<int> max_bp_sizes(arr, arr + sizeof(arr) / sizeof(int));
    std::vector<double> lru_runtimes_workload_1;
    // std::vector<double> lru_runtimes_workload_2;
    std::vector<double> clock_runtimes_workload_1;
    // std::vector<double> clock_runtimes_workload_2;

    size_t n = 256 + 1; // height 2
    int initial_num_bits = 2;
    int maximum_num_items_threshold = 10;


    for (int max_bp_size : max_bp_sizes) {
        cout << "MAX BP SIZE: " << max_bp_size << endl;
        KeyValueStore db_lru = KeyValueStore(n * DB_PAIR_SIZE / 2, "LRU", initial_num_bits, max_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)

        unordered_map<db_key_t, db_val_t> pairs;
        while (pairs.size() < n) {
            db_key_t key = pairs.size();
            db_val_t val = rand();
            pairs[key] = val;
            db_lru.put(key, val);
        }

        auto start = std::chrono::high_resolution_clock::now();
        for (auto pair : pairs) {
            db_key_t key = pair.first;
            db_lru.get(0);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        
        // Calculate duration
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        double runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        lru_runtimes_workload_1.push_back(runtime);

        KeyValueStore db_clock = KeyValueStore(n * DB_PAIR_SIZE / 2, "clock", initial_num_bits, max_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)

        pairs.clear();
        while (pairs.size() < n) {
            db_key_t key = pairs.size();
            db_val_t val = rand();
            pairs[key] = val;
            db_clock.put(key, val);
        }

        start = std::chrono::high_resolution_clock::now();
        for (auto pair : pairs) {
            db_key_t key = pair.first;
            db_clock.get(key);
        }
        stop = std::chrono::high_resolution_clock::now();
        
        // Calculate duration
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        runtime = duration.count() / 1000000.0; // convert to seconds

        // Store the runtime
        clock_runtimes_workload_1.push_back(runtime);
        // cout << "Runtime: " << runtime << endl;
    }

    // Output the runtime
    cout << "LRU (better)" << endl;
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        std::cout << "Maximum BP size: " << max_bp_sizes[i] << "; Time taken: " << lru_runtimes_workload_1[i] << " seconds" << std::endl;
    }
    cout << "Clock (worse)" << endl;
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        std::cout << "Maximum BP size: " << max_bp_sizes[i] << "; Time taken: " << clock_runtimes_workload_1[i] << " seconds" << std::endl;
    }
    // // Write the results to a CSV file
    // std::ofstream put_file("experiments/phase1_put_runtimes.csv");
    // put_file << "Input size,Runtime\n";
    // for (int i = 0; i < input_sizes.size(); ++i) {
    //     put_file << input_sizes[i] << "," << put_runtimes[i] << "\n";
    // }
    // put_file.close();

    return 0;
}

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
    int arr[] = {
        4096  * 1,
        4096  * 2,
        4096  * 3,
        4096  * 4,
        4096  * 5,
        4096  * 6,
        4096  * 7,
        4096  * 8,
        4096  * 9,
        4096  * 10,
        4096  * 11

    };
    std::vector<int> max_bp_sizes(arr, arr + sizeof(arr) / sizeof(int));
    std::vector<double> lru_runtimes_workload_1;
    // std::vector<double> lru_runtimes_workload_2;
    std::vector<double> clock_runtimes_workload_1;
    // std::vector<double> clock_runtimes_workload_2;

//    size_t n = 256 + 1; // height 2
    size_t n = 20 * (256 + 1) + 1; // height 3

    int initial_num_bits = 2;
    int maximum_num_items_threshold = 4;

    vector<string> policies = { "clock", "LRU" };

    for (int max_bp_size : max_bp_sizes) {
        for (string policy : policies) {
            KeyValueStore db_lru = KeyValueStore(n * DB_PAIR_SIZE / 2, policy, initial_num_bits, max_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)

            unordered_map<db_key_t, db_val_t> pairs;
            while (pairs.size() < n) {
                db_key_t key = pairs.size();
                db_val_t val = rand();
                pairs[key] = val;
                db_lru.put(key, val);
            }

            vector<pair<db_key_t, db_val_t>> pairs_vec(pairs.begin(), pairs.end());
            sort(pairs_vec.begin(), pairs_vec.end());

            auto start = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < 30; ++j) {
                for (int i = 0; i < 10; ++i) {
                    db_lru.get(1);
                }

                for (int i = 0; i < 20; ++i) {
                    db_lru.get(256 * i);
                }
            }
            auto stop = std::chrono::high_resolution_clock::now();
            
            // Calculate duration
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            double runtime = duration.count() / 1000000.0; // convert to seconds

            // Store the runtime
            if (policy == "LRU") {
                lru_runtimes_workload_1.push_back(runtime);
            } else {
                clock_runtimes_workload_1.push_back(runtime);
            }
        }
    }

    // Output the runtime
    cout << "LRU (better)" << endl;
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        std::cout << "size: " << max_bp_sizes[i] << "; LRU = " << lru_runtimes_workload_1[i] << "s vs = " << clock_runtimes_workload_1[i] << "s; Difference (LRU - clock): " << lru_runtimes_workload_1[i] - clock_runtimes_workload_1[i]<< std::endl;
    }

    // Write the results to a CSV file
    std::ofstream lru_workload_1_file("experiments/phase2_LRU_workload_1_runtimes.csv");
    lru_workload_1_file << "Maximum Bufferpool Size,Runtime\n";
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        lru_workload_1_file << max_bp_sizes[i] << "," << lru_runtimes_workload_1[i] << "\n";
    }
    lru_workload_1_file.close();

    std::ofstream clock_workload_1_file("experiments/phase2_clock_workload_1_runtimes.csv");
    clock_workload_1_file << "Maximum Bufferpool Size,Runtime\n";
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        clock_workload_1_file << max_bp_sizes[i] << "," << clock_runtimes_workload_1[i] << "\n";
    }
    clock_workload_1_file.close();

    // plot results
    system("python3 experiments/phase2_plot.py");


    return 0;
}

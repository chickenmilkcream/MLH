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
        4096  * 11,
        4096  * 12,
        4096  * 13,
        4096  * 14,
        4096  * 15
    };
    std::vector<int> max_bp_sizes(arr, arr + sizeof(arr) / sizeof(int));
    std::vector<double> lru_runtimes_workload_1;
    std::vector<double> lru_runtimes_workload_2;
    std::vector<double> clock_runtimes_workload_1;
    std::vector<double> clock_runtimes_workload_2;

//    size_t n = 256 + 1; // height 2
    size_t n = 30 * (256 + 1) + 1; // height 3
    int initial_num_bits = 2;
    int maximum_num_items_threshold = 4;
    vector<string> policies = { "clock", "LRU" };

    std::mt19937 gen(69);
    std::discrete_distribution<int> dist({0.9, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01});
    std::uniform_int_distribution<int> uniform_dist(1, n - 1);

    for (int max_bp_size : max_bp_sizes) {
        for (string policy : policies) {
            KeyValueStore db1 = KeyValueStore(n * DB_PAIR_SIZE / 2, policy, initial_num_bits, max_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)
            KeyValueStore db2 = KeyValueStore(n * DB_PAIR_SIZE / 2, policy, initial_num_bits, max_bp_size, maximum_num_items_threshold); // writes two SSTs (+ some in memory)

            unordered_map<db_key_t, db_val_t> pairs;
            while (pairs.size() < n) {
                db_key_t key = pairs.size();
                db_val_t val = rand();
                pairs[key] = val;
                db1.put(key, val);
                db2.put(key, val);
            }

            // vector<pair<db_key_t, db_val_t>> pairs_vec(pairs.begin(), pairs.end());
            // sort(pairs_vec.begin(), pairs_vec.end());

            // Workload 1
            auto start1 = std::chrono::high_resolution_clock::now();
            for (int j = 0; j < 30; ++j) {
                for (int i = 0; i < 10; ++i) {
                    db1.get(1);
                }

                for (int i = 0; i < 20; ++i) {
                    db1.get(256 * i);
                }
            }

            // for (int i = 0; i < 1000; ++i) {
            //     int key = dist(gen) == std::uniform_int_distribution<int>(0, 1000)(gen) ? 1 : std::uniform_int_distribution<int>(1000, n - 1)(gen);
            //     db1.get(key);
            // }
            auto stop1 = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
            double runtime1 = duration1.count() / 1000000.0; // convert to seconds

            // Workload 2
            auto start2 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 1000; ++i) {
                int key = uniform_dist(gen);
                db2.get(key);
            }
            auto stop2 = std::chrono::high_resolution_clock::now();
            auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
            double runtime2 = duration2.count() / 1000000.0; // convert to seconds
            
            // Store the runtime
            if (policy == "LRU") {
                lru_runtimes_workload_1.push_back(runtime1);
                lru_runtimes_workload_2.push_back(runtime2);
            } else {
                clock_runtimes_workload_1.push_back(runtime1);
                clock_runtimes_workload_2.push_back(runtime2);
            }
        }
    }

    // Output the runtime
    cout << "Workload 1: LRU (better)" << endl;
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        std::cout << "size: " << max_bp_sizes[i] << "; LRU = " << lru_runtimes_workload_1[i] << "s vs = " << clock_runtimes_workload_1[i] << "s; Difference (LRU - clock): " << lru_runtimes_workload_1[i] - clock_runtimes_workload_1[i]<< std::endl;
    }
    cout << "Workload 2: CLock (better)" << endl;
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        std::cout << "size: " << max_bp_sizes[i] << "; LRU = " << lru_runtimes_workload_2[i] << "s vs = " << clock_runtimes_workload_2[i] << "s; Difference (clock - LRU): " << clock_runtimes_workload_2[i] - lru_runtimes_workload_2[i]<< std::endl;
    }
    // Write the results to a CSV file
    std::ofstream workload_1_file("experiments/phase2_workload_1_runtimes.csv");
    workload_1_file << "Maximum Bufferpool Size,LRU Runtime,Clock Runtime\n";
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        workload_1_file << max_bp_sizes[i] << "," << lru_runtimes_workload_1[i] << "," << clock_runtimes_workload_1[i] << "\n";
    }
    workload_1_file.close();
    std::ofstream workload_2_file("experiments/phase2_workload_2_runtimes.csv");
    workload_2_file << "Maximum Bufferpool Size,LRU Runtime,Clock Runtime\n";
    for (int i = 0; i < max_bp_sizes.size(); ++i) {
        workload_2_file << max_bp_sizes[i] << "," << lru_runtimes_workload_2[i] << "," << clock_runtimes_workload_2[i] << "\n";
    }
    workload_2_file.close();

    // plot results
    system("python3 experiments/phase2_experiment1_plot.py workload1");
    system("python3 experiments/phase2_experiment1_plot.py workload2");
    return 0;
}

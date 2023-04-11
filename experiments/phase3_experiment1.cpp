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
    std::vector<double> cumulative_put_runtimes;
    std::vector<int> pairs_inserted;

    size_t memtable_size = 10485760;                            // This is 10 MB
    string policy = "clock";
    int initial_num_bits = 2;
    int max_bp_size = 10485760;                                 // This is 10 MB
    int maximum_num_items_threshold = 5000;
    int num_kv_pairs_gigabyte = 5 * 10485760 / (DB_PAIR_SIZE);  // This is 50 MB worth of KV pairs
    int chunk_size = 1048576 / (2 * DB_PAIR_SIZE);              // This is 1 MB worth of KV pairs
    srand(690);

    KeyValueStore db = KeyValueStore(memtable_size, policy, initial_num_bits, max_bp_size, maximum_num_items_threshold);

    for (int i = 0; i < num_kv_pairs_gigabyte; i += chunk_size) {
        pairs_inserted.push_back(i * DB_PAIR_SIZE);

        auto chunk_start_put = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < chunk_size; j++) {
            db_key_t key = i + j;
            db_val_t val = 1;
            db.put(key, val);
        }
        auto chunk_end_put = std::chrono::high_resolution_clock::now();
        auto chunk_time_put = std::chrono::duration_cast<std::chrono::milliseconds>(chunk_end_put - chunk_start_put).count();
        put_runtimes.push_back(chunk_time_put);
        std::cout << "Inserted " << (i) << " key-value pairs or " << (i * 100 / num_kv_pairs_gigabyte) << "% so far, this chunck took " << chunk_time_put << " ms" << std::endl;

        auto chunk_start_get = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < 1000; j++) {
            // get ranom key that is between 0 and i
            db_key_t key = rand() % (i + chunk_size);
            db.get(key);
        }
        auto chunk_end_get = std::chrono::high_resolution_clock::now();
        auto chunk_time_get = std::chrono::duration_cast<std::chrono::milliseconds>(chunk_end_get - chunk_start_get).count();
        get_runtimes.push_back(chunk_time_get);
//        std::cout << "Got key " << 0 << " which took " << chunk_time_get << " ms" << std::endl;

        auto chunk_start_scan = std::chrono::high_resolution_clock::now();
        db.scan(0, num_kv_pairs_gigabyte);
        auto chunk_end_scan = std::chrono::high_resolution_clock::now();
        auto chunk_time_scan = std::chrono::duration_cast<std::chrono::milliseconds>(chunk_end_scan - chunk_start_scan).count();
        scan_runtimes.push_back(chunk_time_scan);
        std::cout << "Scanned all keys which took " << chunk_time_scan << " ms" << std::endl;
    }

    std::cout << "Finished inserting " << num_kv_pairs_gigabyte << " key-value pairs" << std::endl;

    double sum = 0.0;
    for (size_t i = 0; i < put_runtimes.size(); ++i) {
        sum += put_runtimes[i];
        cumulative_put_runtimes.push_back(sum);
    }

    std::ofstream result_file_1;
    result_file_1.open("experiments/phase3_put_runtimes.csv");
    result_file_1 << "Volume of Key-Value Pairs,Cumulative Put Runtime\n";
    for (int i = 0; i < pairs_inserted.size(); ++i) {
        result_file_1 << pairs_inserted[i] << "," << cumulative_put_runtimes[i] << "\n";
    }
    result_file_1.close();

    std::ofstream result_file_2;
    result_file_2.open("experiments/phase3_get_runtimes.csv");
    result_file_2 << "Volume of Key-Value Pairs,Get Runtime\n";
    for (int i = 0; i < pairs_inserted.size(); ++i) {
        result_file_2 << pairs_inserted[i] << "," << get_runtimes[i] << "\n";
    }
    result_file_2.close();

    std::ofstream result_file_3;
    result_file_3.open("experiments/phase3_scan_runtimes.csv");
    result_file_3 << "Volume of Key-Value Pairs,Scan Runtime\n";
    for (int i = 0; i < pairs_inserted.size(); ++i) {
        result_file_3 << pairs_inserted[i] << "," << scan_runtimes[i] << "\n";
    }
    result_file_3.close();
    system("python3 experiments/phase3_experiment1_plot.py");

    return 0;
}

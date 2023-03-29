#include "../kv_store.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include <random>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <unordered_map>
#include <vector>


using namespace std;

int main(int argc, char *argv[]) {
    vector<search_alg> algs = { search_alg::binary_search, search_alg::b_tree_search };
    vector<size_t> nlist = {
        10000,
        20000,
        30000,
        40000,
        50000
    };

    // N = 10000 (binary search): 73.4888
    // N = 10000 (b-tree search): 64.7316
    // N = 20000 (binary search): 107.432
    // N = 20000 (b-tree search): 85.0067
    // N = 30000 (binary search): 123.531
    // N = 30000 (b-tree search): 98.3139
    // N = 40000 (binary search): 148.954
    // N = 40000 (b-tree search): 117.996
    // N = 50000 (binary search): 154.598
    // N = 50000 (b-tree search): 125.24

    std::vector<double> binary_search_runtimes;
    std::vector<double> b_tree_search_runtimes;

    for (size_t n : nlist) {
        for (search_alg alg : algs) {
            KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE / 5, "LRU", 8, 4096, 8);

            // generate uniformly random key-value pairs
            mt19937 gen(443); // seed with 443
            uniform_int_distribution<db_key_t> pdist(DB_KEY_MIN, DB_KEY_MAX); // distribution for putting key-value pairs
            uniform_int_distribution<size_t> gdist(0, n - 1); // distribution for getting keys

            unordered_map<db_key_t, db_val_t> pairs;
            while (pairs.size() < n) {
                db_key_t key = pdist(gen);
                db_val_t val = pdist(gen);
                pairs[key] = val;
            }

            // put key-value pairs into database
            vector<db_key_t> keys;
            for (auto pair : pairs) {
                db_key_t key = pair.first;
                db_val_t val = pair.second;
                db.put(key, val);

                keys.push_back(key);
            }

            auto start = chrono::high_resolution_clock::now();
            // get key-value pairs from database
            for (size_t i = 0; i < 10000; i++) {
                size_t j = gdist(gen);
                db_key_t key = keys[j];
                db.get(key, alg);
            }
            
            // scan key-value pairs from database
            for (size_t i = 0; i < 5000; i++) {
                size_t j = gdist(gen);
                size_t k = gdist(gen);
                db_key_t min_key = min(keys[j], keys[k]);
                db_key_t max_key = max(keys[j], keys[k]);
                db.scan(min_key, max_key, alg);
            }
            auto stop = chrono::high_resolution_clock::now();

            auto duration = chrono::duration_cast<std::chrono::microseconds>(stop - start);
            double runtime = duration.count() / 1000000.0; // convert to seconds

            cout << "N = " << n << (alg == search_alg::binary_search ? " (binary search): " : " (b-tree search): ") << runtime << endl;
        
            if (alg == search_alg::binary_search) {
                binary_search_runtimes.push_back(runtime);
            } else {
                b_tree_search_runtimes.push_back(runtime);
            }
        }
    }

    std::ofstream search_file;
    search_file.open("experiments/phase2_search_runtimes.csv");
    search_file << "Data Size,Binary Search Runtime,B Tree Runtime\n";
    for (int i = 0; i < nlist.size(); ++i) {
        search_file << nlist[i] << "," << binary_search_runtimes[i] << "," << b_tree_search_runtimes[i] << "\n";
    }
    search_file.close();

    // plot results
    system("python3 experiments/phase2_experiment2_plot.py");
}

#include "../kv_store.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <random>
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

    // N = 10000 (binary search): 20.4204
    // N = 10000 (b-tree search): 18.2818
    // N = 20000 (binary search): 48.9458
    // N = 20000 (b-tree search): 36.9872
    // N = 30000 (binary search): 82.8542
    // N = 30000 (b-tree search): 57.0279
    // N = 40000 (binary search): 108.874
    // N = 40000 (b-tree search): 79.048

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
        }
    }
}

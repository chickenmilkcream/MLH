#include "../kv_store.h"

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
    }; // TODO: is there a segfault?

    // N = 10000 (binary search): 20.4204
    // N = 10000 (b-tree search): 18.2818
    // N = 20000 (binary search): 48.9458
    // N = 20000 (b-tree search): 36.9872
    // N = 30000 (binary search): 82.8542
    // N = 30000 (b-tree search): 57.0279
    // N = 40000 (binary search): 108.874
    // N = 40000 (b-tree search): 79.048

    for (size_t n : nlist) {
        for (search_alg alg : algs ) {
            KeyValueStore db = KeyValueStore(n * DB_PAIR_SIZE / 5, "LRU", 8, 4096, 8);

            // generate uniformly random key-value pairs
            random_device rand_dev;
            mt19937 gen(rand_dev());
            uniform_int_distribution<db_key_t> pdist(DB_KEY_MIN, DB_KEY_MAX);
            uniform_int_distribution<size_t> gdist(0, n - 1);

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
            for (size_t i = 0; i < n; i++) {
                size_t j = gdist(gen);
                db_key_t key = keys[j];
                db.get(key, alg);
            }
            // TODO: do some scans too
            auto stop = chrono::high_resolution_clock::now();

            auto duration = chrono::duration_cast<std::chrono::microseconds>(stop - start);
            double runtime = duration.count() / 1000000.0; // convert to seconds

            cout << "N = " << n << (alg == search_alg::binary_search ? " (binary search): " : " (b-tree search): ") << runtime << endl;
        }
    }
}

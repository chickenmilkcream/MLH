#include "kv_store.h"
#include "memtable.h"

#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>

using namespace std;

long nceil(long x, long n) {
    return ceil((double) x / n) * n;
}

long nfloor(long x, long n) {
    return floor((double) x / n) * n;
}

ssize_t aligned_pwrite(int fd, const void *buf, size_t n, off_t offset) {
    size_t size = nceil(n, PAGE_SIZE);
    void *aligned_buf = aligned_alloc(PAGE_SIZE, size);
    for (size_t i = 0; i < n; i++) {
        ((char *) aligned_buf)[i] = ((char *) buf)[i];
    }
    ssize_t n_written = pwrite(fd, aligned_buf, size, offset);
    free(aligned_buf);
    return n_written;
}

ssize_t aligned_pread(int fd, void *buf, size_t n, off_t offset) {
    size_t size = nceil(n, PAGE_SIZE);
    void *aligned_buf = aligned_alloc(PAGE_SIZE, size);
    ssize_t n_read = pread(fd, aligned_buf, size, offset);
    for (size_t i = 0; i < n; i++) {
        ((char *) buf)[i] = ((char *) aligned_buf)[i];
    }
    free(aligned_buf);
    return n_read;
}

KeyValueStore::KeyValueStore(size_t memtable_size)
{
    this->memtable = Memtable(memtable_size);
    this->num_sst = 1;
    this->memtable_size = memtable_size;
}

void KeyValueStore::open_db(string db)
{
    // todo load all SSTs i think...
    // TODO: discuss with team what else to do here
}

void KeyValueStore::close_db()
{
    this->serialize();
    // TODO: discuss with team what else to do here
}

// helper function for KeyValueStore::get
db_val_t KeyValueStore::binary_search(int fd, db_key_t key) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    vector<size_t> sizes;
    off_t offset = this->sizes(fd, sizes);
    size_t height = sizes.size();

    char buf[PAGE_SIZE];

    for (size_t i = 0; i < height - 1; i++) {
        offset += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    off_t start = offset; // offset of level (height - 1)

    // binary search across terminal nodes
    off_t low = start / PAGE_SIZE;
    off_t high = (start + nceil(sizes[height - 1] * DB_PAIR_SIZE, PAGE_SIZE)) / PAGE_SIZE - 1;
    while (low <= high) {
        off_t mid = (low + high) / 2;
    
        offset = mid * PAGE_SIZE;
        aligned_pread(fd, buf, PAGE_SIZE, offset);

        if (((pair<db_key_t, db_val_t> *) buf)[min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1].first < key) {
            low = mid + 1;
        } else if (((pair<db_key_t, db_val_t> *) buf)[0].first > key) {
            high = mid - 1;
        } else {
            break;
        }
    }

    // binary search within terminal node
    ssize_t _low = 0;
    ssize_t _high = min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1;

    while (_low <= _high) {
        ssize_t mid = (_low + _high) / 2;
        if (((pair<db_key_t, db_val_t> *) buf)[mid].first < key) {
            _low = mid + 1;
        } else if (((pair<db_key_t, db_val_t> *) buf)[mid].first > key) {
            _high = mid - 1;
        } else {
            db_key_t val = ((pair<db_key_t, db_val_t> *) buf)[mid].second;
            return val;
        }
    }

    throw invalid_argument("Key not found");
}

// helper function for KeyValueStore::get
db_val_t KeyValueStore::b_tree_search(int fd, db_key_t key) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    vector<size_t> sizes;
    off_t offset = this->sizes(fd, sizes); // offset into file
    off_t offset_buf = 0; // (additional) offset into buffer
    size_t height = sizes.size();

    char buf[PAGE_SIZE];

    aligned_pread(fd, buf, PAGE_SIZE, offset);

    off_t start = offset; // offset of level i

    size_t i = 0;
    while (i < height - 1) {
        // binary search within non-terminal node
        ssize_t low = offset_buf / DB_KEY_SIZE;
        ssize_t high = min(low + b, sizes[i] - (offset - start) / DB_KEY_SIZE) - 1;

        size_t j = high + 1;
        while (low <= high) {
            ssize_t mid = (low + high) / 2;
            if (((db_key_t *) buf)[mid] < key) {
                low = mid + 1;
            } else {
                j = mid;
                high = mid - 1;
            }
        }

        offset = start + nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE) + 
                 (((offset + offset_buf - start) / DB_KEY_SIZE) / b * (b + 1) +
                  j - offset_buf / DB_KEY_SIZE) *
                 b * (i < height - 2 ? DB_KEY_SIZE : DB_PAIR_SIZE);
        offset_buf = offset % PAGE_SIZE;
        offset = nfloor(offset, PAGE_SIZE);
        start += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);

        aligned_pread(fd, buf, PAGE_SIZE, offset);

        i++;
    }

    // binary search within terminal node
    ssize_t _low = 0;
    ssize_t _high = min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1;

    while (_low <= _high) {
        ssize_t mid = (_low + _high) / 2;
        if (((pair<db_key_t, db_val_t> *) buf)[mid].first < key) {
            _low = mid + 1;
        } else if (((pair<db_key_t, db_val_t> *) buf)[mid].first > key) {
            _high = mid - 1;
        } else {
            db_key_t val = ((pair<db_key_t, db_val_t> *) buf)[mid].second;
            return val;
        }
    }

    throw invalid_argument("Key not found");
}

db_val_t KeyValueStore::get(db_key_t key, search_alg alg)
{    
    try {
        // Try to look in the memtable for the key
        db_key_t val = this->memtable.get(key);
        return val;
    } catch (invalid_argument &e) {
        // Couldn't find it in the memtable
        // Loop through the SSTs from latest to oldest
        for (size_t i = this->num_sst - 1; i > 0; i--) {
            const char *filename = ("sst_" + to_string(i) + ".bin").c_str();
            int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);

            db_key_t val;
            if (alg == search_alg::binary_search) {
                try {
                    val = this->binary_search(fd, key);
                } catch (invalid_argument &e) {
                    continue;
                }                
            } else if (alg == search_alg::b_tree_search) {
                try {
                    val = this->b_tree_search(fd, key);
                } catch (invalid_argument &e) {
                    continue;
                }
            } else {
                close(fd);
                throw invalid_argument("Search algorithm not found");
            }

            close(fd);
            return val;
        }

        throw invalid_argument("Key not found");
    }
}

void KeyValueStore::put(db_key_t key, db_val_t val)
{
    this->memtable.put(key, val);

    if (this->memtable.size == this->memtable.max_size)
    {
        this->serialize();
        this->num_sst++;
    }
}

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key, search_alg alg)
{
    vector<pair<db_key_t, db_val_t> > memtable_results = this->memtable.scan(min_key, max_key);
    vector<pair<db_key_t, db_val_t> > sst_results;

    // Loop through the SSTs from latest to oldest
    for (int i = this->num_sst - 1; i > 0; i--)
    {
        // Open the file
        string s = "sst_" + to_string(i) + ".bin";
        int file = open(s.c_str(), O_RDONLY);
        if (file == -1) continue; // If we can't find open the file

        int position = binary_search_smallest(file, min_key);
        if (position == -1) continue; // If we can't find min_key in the file

        // Jump to the position in the file with the min_key
        off_t offset = position * DB_PAIR_SIZE;

        // Continuously scan until we reach max_key or end of file
        while (true)
        {
            pair<db_key_t, db_val_t> kv_pair;
            ssize_t bytes_read = pread(file, &kv_pair, DB_PAIR_SIZE, offset);
            if (kv_pair.first > max_key || bytes_read == 0) break;
            sst_results.push_back(kv_pair);
            offset += DB_PAIR_SIZE;
        }
        close(file);
    }

    // Return the memtable and SST results
    memtable_results.insert(memtable_results.end(), sst_results.begin(), sst_results.end());
    return memtable_results;
}

void KeyValueStore::print() { this->memtable.print(); }

off_t KeyValueStore::sizes(int fd, vector<size_t> &sizes) {
    char buf[PAGE_SIZE];
    off_t offset = 0;

    aligned_pread(fd, buf, PAGE_SIZE, offset);
    offset += PAGE_SIZE;

    size_t i = 0;
    while (((size_t *) buf)[i]) { // read until null terminator
        sizes.push_back(((size_t *) buf)[i]);
        i++;
        if (i == PAGE_SIZE / sizeof(size_t)) {
            aligned_pread(fd, buf, PAGE_SIZE, offset);
            offset += PAGE_SIZE;
            i = 0;
        }
    }

    return offset;
}

void KeyValueStore::write_to_file(const char *filename,
                                  vector<size_t> sizes,
                                  vector<vector<db_key_t> > non_terminal_nodes,
                                  vector<pair<db_key_t, db_val_t> > terminal_nodes)
{
    int fd = open(filename,
                  O_WRONLY | O_DIRECT | O_SYNC | O_TRUNC | O_CREAT,
                  S_IRUSR | S_IWUSR);

    off_t offset = 0;
    size_t height = sizes.size() - 1;

    aligned_pwrite(fd, sizes.data(), (height + 1) * sizeof(size_t), offset);
    offset += nceil((height + 1) * sizeof(size_t), PAGE_SIZE);

    // write levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        aligned_pwrite(fd,
                       non_terminal_nodes[i].data(),
                       sizes[i] * DB_KEY_SIZE,
                       offset);
        offset += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    // write level height - 1
    aligned_pwrite(fd,
                   terminal_nodes.data(),
                   sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>),
                   offset);
    offset += nceil(sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>), PAGE_SIZE);

    close(fd);
}

// for debugging
void KeyValueStore::read_from_file(const char *filename)
{
    vector<size_t> sizes;
    vector<vector<db_key_t> > non_terminal_nodes;
    vector<pair<db_key_t, db_val_t> > terminal_nodes;

    int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);

    off_t offset = this->sizes(fd, sizes);
    size_t height = sizes.size();

    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    // read levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes[i].resize(sizes[i]);
        aligned_pread(fd,
                      non_terminal_nodes[i].data(),
                      sizes[i] * DB_KEY_SIZE,
                      offset);
        offset += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    // read level height - 1
    terminal_nodes.resize(sizes[height - 1]);
    aligned_pread(fd,
                  terminal_nodes.data(),
                  sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>),
                  offset);
    offset += nceil(sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>), PAGE_SIZE);
    
    close(fd);

    // print...
    for (size_t i = 0; i < height; i++) {
        cout << "level " << i << " size: " << sizes[i] << endl;
    }

    if (height > 1) {
        for (size_t i = 0; i < height - 1; i++) {
            cout << "[";
            for (size_t j = 0; j < sizes[i]; j++) {
                if (j == 16) {
                    cout << "...";
                    break;
                }
                cout << non_terminal_nodes[i][j];
                if (j < sizes[i] - 1) {
                    cout << ", ";
                }
            }
            cout << "]" << endl;
        }        
    }

    cout << "[";
    for (size_t i = 0; i < sizes[height - 1]; i++) {
        if (i == 16) {
            cout << "...";
            break;
        }
        cout << terminal_nodes[i].first << ":" << terminal_nodes[i].second;
        if (i < sizes[height - 1] - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

void KeyValueStore::serialize()
{
    vector<pair<db_key_t, db_val_t> > pairs = this->memtable.scan(DB_KEY_MIN, DB_KEY_MAX);
    this->memtable = Memtable(this->memtable_size);

    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    size_t height = ceil(log((double) pairs.size() / b) / log(b + 1)) + 1;

    // populate levels 0 - (height - 2)
    vector<vector<db_key_t> > non_terminal_nodes;
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    size_t i = b - 1;
    while (i < pairs.size()) {
        for (size_t j = height; j > 0; j--) {
            if ((i + 1) % ((size_t) pow(b + 1, j - 1) * b) == 0) {
                if (j < height) {
                    non_terminal_nodes[height - j - 1].push_back(pairs[i].first);
                }
                break;
            }
        }
        i += b;
    }

    // populate level (height - 1)
    vector<pair<db_key_t, db_val_t> > terminal_nodes = pairs;
    
    vector<size_t> sizes;
    for (size_t i = 0; i < height - 1; i++) {
        size_t size = non_terminal_nodes[i].size();
        sizes.push_back(size);
    }
    size_t size = terminal_nodes.size();
    sizes.push_back(size);
    sizes.push_back(0); // write null terminator

    const char *filename = ("sst_" + to_string(this->num_sst) + ".bin").c_str();
    this->write_to_file(filename, sizes, non_terminal_nodes, terminal_nodes);
}

int KeyValueStore::binary_search_smallest(int file, db_key_t target)
{
    // This function finds the position of the smallest key that is greater than target

    // Compute the number of key-value pairs in the file
    int numPairs = lseek(file, 0, SEEK_END) / DB_PAIR_SIZE;

    // Binary search loop
    int low = 0, high = numPairs - 1, result = numPairs;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        pair<db_key_t, db_val_t> pair;
        pread(file, &pair, DB_PAIR_SIZE, mid * DB_PAIR_SIZE);

        if (pair.first >= target)
        {
            result = mid;
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return result;
}

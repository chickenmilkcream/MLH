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

// helper function for KeyValueStore::get and KeyValueStore::scan
void KeyValueStore::binary_search(int fd, db_key_t key, vector<size_t> sizes, size_t height, off_t &start, off_t &offset) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    start = offset;
    for (size_t i = 0; i < height - 1; i++) {
        start += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    char buf[PAGE_SIZE];

    // binary search across terminal nodes
    ssize_t low = start / PAGE_SIZE;
    ssize_t high = (start + nceil(sizes[height - 1] * DB_PAIR_SIZE, PAGE_SIZE)) / PAGE_SIZE - 1;

    size_t i = high + 1; // think about this init more...
    while (low <= high) {
        ssize_t mid = (low + high) / 2;

        offset = mid * PAGE_SIZE;
        aligned_pread(fd, buf, PAGE_SIZE, offset);

        if (((pair<db_key_t, db_val_t> *) buf)[min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1].first < key) {
            low = mid + 1;
        } else {
            i = mid;
            high = mid - 1;
        }
    }

    offset = i * PAGE_SIZE;
}

// helper function for KeyValueStore::get and KeyValueStore::scan
void KeyValueStore::b_tree_search(int fd, db_key_t key, vector<size_t> sizes, size_t height, off_t &start, off_t &offset) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    start = offset;

    char buf[PAGE_SIZE];
    aligned_pread(fd, buf, PAGE_SIZE, offset);

    off_t offset_buf = 0;

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
}

db_val_t KeyValueStore::get(db_key_t key, search_alg alg)
{    
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

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
            off_t start = 0;
            off_t offset = 0;

            vector<size_t> sizes;
            size_t height;
            this->sizes(fd, offset, sizes, height);

            if (alg == search_alg::binary_search) {
                this->binary_search(fd, key, sizes, height, start, offset);
            } else if (alg == search_alg::b_tree_search) {
                this->b_tree_search(fd, key, sizes, height, start, offset);
            } else {
                close(fd);
                throw invalid_argument("Search algorithm not found");
            }

            char buf[PAGE_SIZE];
            aligned_pread(fd, buf, PAGE_SIZE, offset);
            close(fd);

            // binary search within terminal node
            ssize_t low = 0;
            ssize_t high = min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1;

            while (low <= high) {
                ssize_t mid = (low + high) / 2;
                if (((pair<db_key_t, db_val_t> *) buf)[mid].first < key) {
                    low = mid + 1;
                } else if (((pair<db_key_t, db_val_t> *) buf)[mid].first > key) {
                    high = mid - 1;
                } else {
                    db_key_t val = ((pair<db_key_t, db_val_t> *) buf)[mid].second;
                    return val;
                }
            }
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
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    vector<pair<db_key_t, db_val_t> > pairs = this->memtable.scan(min_key, max_key);
    
    // Loop through the SSTs from latest to oldest
    for (size_t i = this->num_sst - 1; i > 0; i--) {
        const char *filename = ("sst_" + to_string(i) + ".bin").c_str();
        int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
        off_t start = 0;
        off_t offset = 0;

        vector<size_t> sizes;
        size_t height;
        this->sizes(fd, offset, sizes, height);

        if (alg == search_alg::binary_search) {
            this->binary_search(fd, min_key, sizes, height, start, offset);
        } else if (alg == search_alg::b_tree_search) {
            this->b_tree_search(fd, min_key, sizes, height, start, offset);
        } else {
            close(fd);
            throw invalid_argument("Search algorithm not found");
        }

        char buf[PAGE_SIZE];
        aligned_pread(fd, buf, PAGE_SIZE, offset);

        // binary search within terminal node
        ssize_t low = 0;
        ssize_t high = min(b, sizes[height - 1] - (offset - start) / DB_PAIR_SIZE) - 1;

        size_t j = 0; // TODO: think about this init more..., offset vs fd?, simplify intermediate calcs..
        while (low <= high) {
            ssize_t mid = (low + high) / 2;
            if (((pair<db_key_t, db_val_t> *) buf)[mid].first < min_key) {
                low = mid + 1;
            } else {
                j = mid;
                high = mid - 1;                
            }
        }

        size_t k = j;
        while (((pair<db_key_t, db_val_t> *) buf)[k].first <= max_key && k < sizes[height - 1] - (offset - PAGE_SIZE - start) / DB_PAIR_SIZE) {
            pairs.push_back(((pair<db_key_t, db_val_t> *) buf)[k]);
            k++;
            if (k == b) {
                aligned_pread(fd, buf, PAGE_SIZE, offset);
                offset += PAGE_SIZE;
                k = 0;
            }
        }

        close(fd);
    }

    return pairs;
}

void KeyValueStore::print() { this->memtable.print(); }

void KeyValueStore::sizes(int fd, off_t &fp, vector<size_t> &sizes, size_t &height) {
    char buf[PAGE_SIZE];
    fp = 0;

    aligned_pread(fd, buf, PAGE_SIZE, fp);
    fp += PAGE_SIZE;

    size_t i = 0;
    while (((size_t *) buf)[i]) { // read until null terminator
        sizes.push_back(((size_t *) buf)[i]);
        i++;
        if (i == PAGE_SIZE / sizeof(size_t)) {
            aligned_pread(fd, buf, PAGE_SIZE, fp);
            fp += PAGE_SIZE;
            i = 0;
        }
    }

    height = sizes.size();
}

void KeyValueStore::write_to_file(const char *filename,
                                  vector<size_t> sizes,
                                  vector<vector<db_key_t> > non_terminal_nodes,
                                  vector<pair<db_key_t, db_val_t> > terminal_nodes)
{
    int fd = open(filename,
                  O_WRONLY | O_DIRECT | O_SYNC | O_TRUNC | O_CREAT,
                  S_IRUSR | S_IWUSR);

    off_t fp = 0;
    size_t height = sizes.size() - 1;

    aligned_pwrite(fd, sizes.data(), (height + 1) * sizeof(size_t), fp);
    fp += nceil((height + 1) * sizeof(size_t), PAGE_SIZE);

    // write levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        aligned_pwrite(fd,
                       non_terminal_nodes[i].data(),
                       sizes[i] * DB_KEY_SIZE,
                       fp);
        fp += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    // write level height - 1
    aligned_pwrite(fd,
                   terminal_nodes.data(),
                   sizes[height - 1] * DB_PAIR_SIZE,
                   fp);

    close(fd);
}

// for debugging
void KeyValueStore::read_from_file(const char *filename)
{
    vector<size_t> sizes;
    vector<vector<db_key_t> > non_terminal_nodes;
    vector<pair<db_key_t, db_val_t> > terminal_nodes;

    int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
    off_t fp;

    size_t height;
    this->sizes(fd, fp, sizes, height);

    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    // read levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes[i].resize(sizes[i]);
        aligned_pread(fd,
                      non_terminal_nodes[i].data(),
                      sizes[i] * DB_KEY_SIZE,
                      fp);
        fp += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    // read level height - 1
    terminal_nodes.resize(sizes[height - 1]);
    aligned_pread(fd,
                  terminal_nodes.data(),
                  sizes[height - 1] * DB_PAIR_SIZE,
                  fp);
    
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

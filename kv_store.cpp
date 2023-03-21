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

ssize_t aligned_pwrite(int fd, const void *buf, size_t n, off_t offset) {
    ssize_t size = ceil((double) n / PAGE_SIZE) * PAGE_SIZE;
    void *aligned_buf = aligned_alloc(PAGE_SIZE, size);
    for (size_t i = 0; i < n; i++) {
        ((char *) aligned_buf)[i] = ((char *) buf)[i];
    }
    ssize_t n_written = pwrite(fd, aligned_buf, size, offset);
    free(aligned_buf);
    return n_written;
}

ssize_t aligned_pread(int fd, void *buf, size_t n, off_t offset) {
    size_t size = ceil((double) n / PAGE_SIZE) * PAGE_SIZE;
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

db_val_t KeyValueStore::get(db_key_t key, search_alg alg)
{
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));

    try {
        // Try to look in the memtable for the key
        return this->memtable.get(key);
    } catch (invalid_argument &e) {
        if (alg == binary_search) {
            // // Couldn't find it in the memtable
            // // Loop through the SSTs from latest to oldest
            // for (int i = this->num_sst - 1; i > 0; i--)
            // {
            //     // Open the file
            //     const char *filename = ("sst_" + to_string(i) + ".bin").c_str();
            //     int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
            //     if (fd == -1) continue; // If we can't find open the file

            //     int position = binary_search_exact(fd, key);
            //     if (position == -1)
            //         continue; // If we can't find min_key in the file

            //     // Jump to the position in the file with the key
            //     off_t offset = position * pairSize;
            //     pair<db_key_t, db_val_t> kv_pair;
            //     pread(fd, &kv_pair, pairSize, offset);

            //     return kv_pair.second;
            // }

            // // After going through all the SSTs, we still can't find it
            throw invalid_argument("Key not found");
        } else if (alg == b_tree_search) {
            size_t b = PAGE_SIZE / sizeof(pair<db_key_t, db_val_t>);

            for (int i = this->num_sst - 1; i > 0; i--) {
                vector<size_t> sizes;

                const char *filename = ("sst_" + to_string(i) + ".bin").c_str();
                int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
                off_t offset = 0;

                char buf[PAGE_SIZE];
                aligned_pread(fd, buf, PAGE_SIZE, offset);
                offset += PAGE_SIZE;
                size_t j = 0;
                while (((size_t *) buf)[j]) { // read until null terminator
                    sizes.push_back(((size_t *) buf)[j]);
                    j++;
                    if (j == PAGE_SIZE / sizeof(size_t)) {
                        aligned_pread(fd, buf, PAGE_SIZE, offset);
                        offset += PAGE_SIZE;
                        j = 0;
                    }
                }

                size_t height = sizes.size();
                off_t start = offset;

                aligned_pread(fd, buf, PAGE_SIZE, offset);
                cout << "reading page: " << offset / PAGE_SIZE << endl;
                cout << "[";
                for (size_t p = 0; p < b; p++) {
                    cout << ((db_key_t *) buf)[p];
                    if (j < sizes[i] - 1) {
                        cout << ", ";
                    }
                }
                cout << "]" << endl;

                off_t remainder = 0;

                j = 0;
                while (j < height - 1) {
                    size_t k = remainder;
                    cout << "offset into page: " << k << endl;

                    // TODO: change to in-memory binary search
                    while (k < remainder + min(b, sizes[j] - (offset - start) / sizeof(db_key_t))
                           && ((db_key_t *) buf)[k] < key) {
                        k++;
                    }

                    cout << "the other min thing: " << sizes[j] - (offset - start) / sizeof(db_key_t) << endl;

                    cout << "j: " << j << endl;
                    cout << "k: " << k << endl;
                    cout << "sizes[j]: " << sizes[j] << endl;

                    off_t end = start + ceil((double) sizes[j] * sizeof(db_key_t) / PAGE_SIZE) * PAGE_SIZE;
                    off_t m = end + ((offset - start) / sizeof(db_key_t) + k) * (b + 1) * (j < height - 2 ? sizeof(db_key_t) : sizeof(pair<db_key_t, db_val_t>));
                    offset = m / PAGE_SIZE * PAGE_SIZE; // round down
                    remainder = m % PAGE_SIZE / sizeof(db_key_t);
                    start = end;

                    cout << "m: " << m << endl;
                    cout << "reading page: " << offset / PAGE_SIZE << endl;
                    aligned_pread(fd, buf, PAGE_SIZE, offset);
                    j++;

                if (j == height - 1) {
                    cout << "[";
                    for (size_t p = 0; p < b; p++) {
                        cout << ((pair<db_key_t, db_val_t> *) buf)[p].first << ":" << ((pair<db_key_t, db_val_t> *) buf)[p].second;
                        if (j < sizes[i] - 1) {
                            cout << ", ";
                        }
                    }
                    cout << "]" << endl;                    
                } else {
                    cout << "[";
                    for (size_t p = 0; p < b; p++) {
                        cout << ((db_key_t *) buf)[p];
                        if (j < sizes[i] - 1) {
                            cout << ", ";
                        }
                    }
                    cout << "]" << endl;
                }

                }

                int k = 0;
                while (k < b && ((pair<db_key_t, db_val_t> *) buf)[k].first != key) {
                    k++;
                }

                if (k == b) {
                    continue;
                };

                db_key_t val = ((pair<db_key_t, db_val_t> *) buf)[k].second;
                return val;
            }
            
            throw invalid_argument("Key not found");
        } else {
            throw invalid_argument("Search algorithm does not exist");
        }
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

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key)
{
    vector<pair<db_key_t, db_val_t> > memtable_results = this->memtable.scan(min_key, max_key);
    vector<pair<db_key_t, db_val_t> > sst_results;
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));

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
        off_t offset = position * pairSize;

        // Continuously scan until we reach max_key or end of file
        while (true)
        {
            pair<db_key_t, db_val_t> kv_pair;
            ssize_t bytes_read = pread(file, &kv_pair, pairSize, offset);
            if (kv_pair.first > max_key || bytes_read == 0) break;
            sst_results.push_back(kv_pair);
            offset += pairSize;
        }
        close(file);
    }

    // Return the memtable and SST results
    memtable_results.insert(memtable_results.end(), sst_results.begin(), sst_results.end());
    return memtable_results;
}

void KeyValueStore::print() { this->memtable.print(); }

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
    offset += ceil((double) (height + 1) * sizeof(size_t) / PAGE_SIZE) * PAGE_SIZE;

    // write levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        aligned_pwrite(fd,
                       non_terminal_nodes[i].data(),
                       sizes[i] * sizeof(db_key_t),
                       offset);
        offset += ceil((double) sizes[i] * sizeof(db_key_t) / PAGE_SIZE) * PAGE_SIZE;
    }

    // write level height - 1
    aligned_pwrite(fd,
                   terminal_nodes.data(),
                   sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>),
                   offset);
    offset += ceil((double) sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>) / PAGE_SIZE) * PAGE_SIZE;

    close(fd);
}

// for debugging
void KeyValueStore::read_from_file(const char *filename)
{
    vector<size_t> sizes;
    vector<vector<db_key_t> > non_terminal_nodes;
    vector<pair<db_key_t, db_val_t> > terminal_nodes;

    int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
    off_t offset = 0;

    char buf[PAGE_SIZE];
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

    size_t height = sizes.size();
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    // read levels 0 - (height - 2)
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes[i].resize(sizes[i]);
        aligned_pread(fd,
                      non_terminal_nodes[i].data(),
                      sizes[i] * sizeof(db_key_t),
                      offset);
        offset += ceil((double) sizes[i] * sizeof(db_key_t) / PAGE_SIZE) * PAGE_SIZE;
    }

    // read level height - 1
    terminal_nodes.resize(sizes[height - 1]);
    aligned_pread(fd,
                  terminal_nodes.data(),
                  sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>),
                  offset);
    offset += ceil((double) sizes[height - 1] * sizeof(pair<db_key_t, db_val_t>) / PAGE_SIZE) * PAGE_SIZE;
    
    close(fd);

    // print...
    for (size_t i = 0; i < height; i++) {
        cout << "level " << i << " size: " << sizes[i] << endl;
    }

    if (height > 1) {
        for (size_t i = 0; i < height - 1; i++) {
            cout << "[";
            for (size_t j = 0; j < sizes[i]; j++) {
                cout << non_terminal_nodes[i][j];
                if (j < sizes[i] - 1) {
                    cout << ", ";
                }
                if (j == 10) {
                    cout << "...";
                    break;
                }
            }
            cout << "]" << endl;
        }        
    }

    cout << "[";
    for (size_t i = 0; i < sizes[height - 1]; i++) {
        cout << terminal_nodes[i].first << ":" << terminal_nodes[i].second;
        if (i < sizes[height - 1] - 1) {
            cout << ", ";
        }
        if (i == 10) {
            cout << "...";
            break;
        }
    }
    cout << "]" << endl;
}

void KeyValueStore::serialize()
{
    vector<pair<db_key_t, db_val_t> > pairs = this->memtable.scan(DB_KEY_MIN, DB_KEY_MAX);
    this->memtable = Memtable(this->memtable_size);

    size_t n = pairs.size();
    size_t b = PAGE_SIZE / sizeof(pair<db_key_t, db_val_t>);

    size_t height = ceil(log((double) n / b) / log(b + 1)) + 1;

    // populate levels 0 - (height - 2)
    vector<vector<db_key_t> > non_terminal_nodes;
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    size_t i = b - 1;
    while (i < n) {
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
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int numPairs = lseek(file, 0, SEEK_END) / pairSize;

    // Binary search loop
    int low = 0, high = numPairs - 1, result = numPairs;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        pair<db_key_t, db_val_t> pair;
        pread(file, &pair, pairSize, mid * pairSize);

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

int KeyValueStore::binary_search_exact(int file, db_key_t target)
{
    // This function finds the position of the key that is equal to target

    // Compute the number of key-value pairs in the file
    int pairSize = (sizeof(db_key_t) + sizeof(db_val_t));
    int numPairs = lseek(file, 0, SEEK_END) / pairSize;

    // Binary search loop
    int low = 0, high = numPairs - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        
        pair<db_key_t, db_val_t> pair;
        pread(file, &pair, pairSize, mid * pairSize);

        if (pair.first == target)
        {
            return mid;
        }
        else if (pair.first < target)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return -1;
}

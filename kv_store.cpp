#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <sys/stat.h>
#include <fstream>
#include <set>
#include <filesystem>

#include "kv_store.h"

using namespace std;

long nceil(long x, long n) {
    return ceil((double) x / n) * n;
}

long nfloor(long x, long n) {
    return floor((double) x / n) * n;
}

bool exists(string filename) {
    struct stat sb;
    return stat(filename.c_str(), &sb) == 0;
}

ssize_t aligned_pwrite(int fd, const void *buf, size_t n, off_t fp) {
    size_t size = nceil(n, PAGE_SIZE);
    void *aligned_buf = aligned_alloc(PAGE_SIZE, size);
    memset(aligned_buf, 0, size);
    memcpy(aligned_buf, buf, n);
    ssize_t n_written = pwrite(fd, aligned_buf, size, fp); // fp must be page-aligned
    free(aligned_buf);
    return n_written;
}

ssize_t aligned_pread(int fd, void *buf, size_t n, off_t fp) {
    size_t size = nceil(n, PAGE_SIZE);
    void *aligned_buf = aligned_alloc(PAGE_SIZE, size);
    memset(aligned_buf, 0, size);
    ssize_t n_read = pread(fd, aligned_buf, size, fp); // fp must be page-aligned
    memcpy(buf, aligned_buf, n);
    free(aligned_buf);
    return n_read;
}

KeyValueStore::KeyValueStore(size_t memtable_size,
                             string eviction_policy,
                             int initial_num_bits,
                             int maximum_bp_size,
                             int maximum_num_items_threshold)
{
    this->memtable = Memtable(memtable_size);
    this->memtable_size = memtable_size;
    this->buffer_pool = BPDirectory(eviction_policy, initial_num_bits, maximum_bp_size, maximum_num_items_threshold);
}

void KeyValueStore::bpread(string filename, int fd, void *buf, off_t fp) {
    // aligned_pread(fd, buf, PAGE_SIZE, fp);

    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page
    try {
        void *page = this->buffer_pool.get_page(filename, fp / PAGE_SIZE)->page_content;
        memcpy(buf, page, PAGE_SIZE);
    } catch (out_of_range &e) {
        aligned_pread(fd, buf, PAGE_SIZE, fp);
        this->buffer_pool.insert_page(buf, b, filename, fp / PAGE_SIZE);
    }
}

void KeyValueStore::open_db(string db)
{
    // TODO TEAM: clarify load all SSTs??
    // TODO TEAM: discuss with team what else to do here
}

void KeyValueStore::close_db()
{
    this->serialize();
    this->buffer_pool.free_all_pages();
}

/*
 * helper function for KeyValueStore::get and KeyValueStore::scan
 * assigns fp to the offset of the page containing key
 */ 
void KeyValueStore::binary_search(string filename, int fd, db_key_t key, vector<size_t> sizes, size_t height, off_t &start, off_t &fp) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    start = fp;
    for (size_t i = 0; i < height - 1; i++) {
        start += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);
    }

    char buf[PAGE_SIZE];

    // binary search in storage
    ssize_t low = start / PAGE_SIZE;
    ssize_t high = (start + nceil(sizes[height - 1] * DB_PAIR_SIZE, PAGE_SIZE)) / PAGE_SIZE - 1;

    while (low <= high) {
        ssize_t mid = (low + high) / 2;

        fp = mid * PAGE_SIZE;
        this->bpread(filename, fd, buf, fp);

        if (((pair<db_key_t, db_val_t> *) buf)[min(b, sizes[height - 1] - (fp - start) / DB_PAIR_SIZE) - 1].first < key) {
            low = mid + 1;
        } else if (((pair<db_key_t, db_val_t> *) buf)[0].first > key) {
            high = mid - 1;
        } else {
            break;
        }
    }
}

/*
 * helper function for KeyValueStore::get and KeyValueStore::scan
 * assigns fp to the offset of the page containing key
 */ 
void KeyValueStore::b_tree_search(string filename, int fd, db_key_t key, vector<size_t> sizes, size_t height, off_t &start, off_t &fp) {
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    start = fp;

    char buf[PAGE_SIZE];
    this->bpread(filename, fd, buf, fp);

    off_t offset = fp; // fp is page-aligned but offset is not stricly page-aligned
    size_t i = 0;
    while (i < height - 1) {
        // binary search in memory
        ssize_t low = (offset - fp) / DB_KEY_SIZE;
        ssize_t high = min(low + b, sizes[i] - (fp - start) / DB_KEY_SIZE) - 1;

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
                 (((offset - start) / DB_KEY_SIZE) / b * (b + 1) + j - 
                  (offset - fp) / DB_KEY_SIZE) * b * 
                 (i < height - 2 ? DB_KEY_SIZE : DB_PAIR_SIZE);
        fp = nfloor(offset, PAGE_SIZE);
        start += nceil(sizes[i] * DB_KEY_SIZE, PAGE_SIZE);

        this->bpread(filename, fd, buf, fp);
        i++;
    }
}

db_val_t KeyValueStore::get(db_key_t key, search_alg alg)
{    
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    try {
        // Try to look in the memtable for the key
        db_key_t val = this->memtable.get(key);
        if (val == DB_TOMBSTONE) {
            throw invalid_argument("Key not found");
        }
        return val;
    } catch (invalid_argument &e) {
        // Couldn't find it in the memtable
        // Loop through the SSTs from latest to oldest
        for (size_t i = 1; i < DB_MAX_LEVEL; i++) {
            string filename = "sst." + to_string(i) + ".1.bin";
            if (exists(filename)) {
                int fd = open(filename.c_str(), O_RDONLY | O_DIRECT | O_SYNC);
                off_t start = 0;
                off_t fp = 0;

                vector<size_t> sizes;
                size_t height;
                this->sizes(filename, fd, fp, sizes, height);

                if (alg == search_alg::binary_search) {
                    this->binary_search(filename, fd, key, sizes, height, start, fp);
                } else if (alg == search_alg::b_tree_search) {
                    this->b_tree_search(filename, fd, key, sizes, height, start, fp);
                } else {
                    close(fd);
                    throw invalid_argument("Search algorithm not found");
                }

                char buf[PAGE_SIZE];
                this->bpread(filename, fd, buf, fp);
                close(fd);

                // binary search in memory
                ssize_t low = 0;
                ssize_t high = min(b, sizes[height - 1] - (fp - start) / DB_PAIR_SIZE) - 1;

                while (low <= high) {
                    ssize_t mid = (low + high) / 2;
                    if (((pair<db_key_t, db_val_t> *) buf)[mid].first < key) {
                        low = mid + 1;
                    } else if (((pair<db_key_t, db_val_t> *) buf)[mid].first > key) {
                        high = mid - 1;
                    } else {
                        db_key_t val = ((pair<db_key_t, db_val_t> *) buf)[mid].second;
                        if (val == DB_TOMBSTONE) {
                            throw invalid_argument("Key not found");
                        }
                        return val;
                    }
                }
            }
        }

        throw invalid_argument("Key not found");
    }
}

void KeyValueStore::put(db_key_t key, db_val_t val)
{   
    if (val == DB_TOMBSTONE) {
        throw invalid_argument("Value not allowed");
    }

    this->memtable.put(key, val);

    if (this->memtable.size + DB_PAIR_SIZE > this->memtable.max_size)
    {
        this->serialize();
    }
}

void KeyValueStore::del(db_key_t key)
{
    this->memtable.put(key, DB_TOMBSTONE);

    if (this->memtable.size + DB_PAIR_SIZE > this->memtable.max_size)
    {
        this->serialize();
    }
}

vector<pair<db_key_t, db_val_t> > KeyValueStore::scan(db_key_t min_key, db_key_t max_key, search_alg alg)
{
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page
    set<db_key_t> deleted_keys;

    if (min_key > max_key) {
        return {};
    }
    vector<pair<db_key_t, db_val_t> > pairs = this->memtable.scan(min_key, max_key, &deleted_keys);
    
    // Loop through the SSTs from latest to oldest
    for (int i = 1; i < DB_MAX_LEVEL; i++) {
        const char *filename = ("sst." + to_string(i) + ".1.bin").c_str();
        int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
        if (fd == -1) continue;
        off_t start = 0;
        off_t fp = 0;

        vector<size_t> sizes;
        size_t height;
        this->sizes(filename, fd, fp, sizes, height);

        if (alg == search_alg::binary_search) {
            this->binary_search(filename, fd, min_key, sizes, height, start, fp);
        } else if (alg == search_alg::b_tree_search) {
            this->b_tree_search(filename, fd, min_key, sizes, height, start, fp);
        } else {
            close(fd);
            throw invalid_argument("Search algorithm not found");
        }

        char buf[PAGE_SIZE];
        this->bpread(filename, fd, buf, fp);
        fp += PAGE_SIZE;

        // binary search in memory (for key greater than or equal to min_key)
        ssize_t low = 0;
        ssize_t high = min(b, sizes[height - 1] - (fp - PAGE_SIZE - start) / DB_PAIR_SIZE) - 1;
        
        size_t j = high + 1;
        while (low <= high) {
            ssize_t mid = (low + high) / 2;
            if (((pair<db_key_t, db_val_t> *) buf)[mid].first < min_key) {
                low = mid + 1;
            } else {
                j = mid;
                high = mid - 1;                
            }
        }

        // scan until key is greater than max_key or EOF is reached
        size_t k = j;
        while (k < min(b, sizes[height - 1] - (fp - PAGE_SIZE - start) / DB_PAIR_SIZE) &&
               ((pair<db_key_t, db_val_t> *) buf)[k].first <= max_key) {
            db_key_t curr_key = ((pair<db_key_t, db_val_t> *) buf)[k].first;
            db_key_t curr_val = ((pair<db_key_t, db_val_t> *) buf)[k].second;
            if (curr_val == DB_TOMBSTONE) {
                deleted_keys.insert(curr_key);
            } else if (!deleted_keys.count(curr_key)) {
                pairs.push_back(((pair<db_key_t, db_val_t> *) buf)[k]);
            }
            k++;
            if (k == b) {
                this->bpread(filename, fd, buf, fp);
                fp += PAGE_SIZE;
                k = 0;
            }
        }

        close(fd);
    }

    return pairs;
}

void KeyValueStore::print() { this->memtable.print(); }

void KeyValueStore::sizes(string filename, int fd, off_t &fp, vector<size_t> &sizes, size_t &height) {
    fp = 0;

    char buf[PAGE_SIZE];
    this->bpread(filename, fd, buf, fp);
    fp += PAGE_SIZE;

    size_t i = 0;
    while (((size_t *) buf)[i]) { // read until null terminator
        sizes.push_back(((size_t *) buf)[i]);
        i++;
        if (i == PAGE_SIZE / sizeof(size_t)) {
            this->bpread(filename, fd, buf, fp);
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
    // cout << "writing..." << filename << endl;
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

void KeyValueStore::write_to_file(const char *filename,
                                  vector<size_t> sizes,
                                  vector<vector<db_key_t> > non_terminal_nodes,
                                  const char *terminal_nodes)
{
    // cout << "writing..." << filename << endl;
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
    int fd_terminal_nodes = open(terminal_nodes, O_RDONLY | O_DIRECT | O_SYNC);
    off_t fp_terminal_nodes = 0;

    char buf[PAGE_SIZE];

    while (fp_terminal_nodes < nceil(sizes[height - 1] * DB_PAIR_SIZE, PAGE_SIZE)) {
        aligned_pread(fd_terminal_nodes, buf, PAGE_SIZE, fp_terminal_nodes);
        fp_terminal_nodes += PAGE_SIZE;
        aligned_pwrite(fd, buf, PAGE_SIZE, fp);
        fp += PAGE_SIZE;
    }

    close(fd);
}

// for debugging
void KeyValueStore::read_from_file(const char *filename)
{
    if (!exists(filename)) {
        throw invalid_argument("File not found");
    }

    vector<size_t> sizes;
    vector<vector<db_key_t> > non_terminal_nodes;
    vector<pair<db_key_t, db_val_t> > terminal_nodes;

    int fd = open(filename, O_RDONLY | O_DIRECT | O_SYNC);
    off_t fp = 0;

    size_t height;
    this->sizes(filename, fd, fp, sizes, height);

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

void KeyValueStore::compact_files(vector<string> filenames) // filenames are ordered from NEWEST to OLDEST
{
    size_t size;
    vector<db_key_t> fence_keys;
    this->compact_files_first_pass(filenames, size, fence_keys);
    this->compact_files_second_pass(filenames, size, fence_keys);
}

// first pass does compaction
void KeyValueStore::compact_files_first_pass(vector<string> filenames,
                                             size_t &size,
                                             vector<db_key_t> &fence_keys)
{
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    // input buffers
    char buf_in[2][PAGE_SIZE];
    
    // output buffer
    char buf_out[PAGE_SIZE];

    // open input files
    int fd_in[2];
    for (size_t i = 0; i < 2; i++) {
        fd_in[i] = open(filenames[i].c_str(), O_RDONLY | O_DIRECT | O_SYNC);
    }

    // open output file
    int fd_out = open("temp",
                      O_WRONLY | O_DIRECT | O_SYNC | O_TRUNC | O_CREAT,
                      S_IRUSR | S_IWUSR);

    // offset of terminal nodes in input files
    off_t start_in[2];

    // offset of terminal nodes in output file
    off_t start_out = 0;

    // page-aligned offsets in input files for reads/writes
    off_t fp_in[2] = {0};

    // page-aligned offset in output file for reads/writes
    off_t fp_out = 0;

    vector<size_t> sizes[2];
    size_t height[2];
    for (size_t i = 0; i < 2; i++) {
        this->sizes(filenames[i], fd_in[i], fp_in[i], sizes[i], height[i]);
        start_in[i] = fp_in[i];
        for (size_t j = 0; j < height[i] - 1; j++) {
            start_in[i] += nceil(sizes[i][j] * DB_KEY_SIZE, PAGE_SIZE);
        }
        fp_in[i] = start_in[i];
    }

    // similar to fp_in, but not necessarily page-aligned
    off_t offset_in[2];
    memcpy(offset_in, start_in, 2 * sizeof(off_t));

    // similar to fp_out, but not necessarily page-aligned
    off_t offset_out = start_out;

    // read pages from input files into input buffers
    for (size_t i = 0; i < 2; i++) {
        this->bpread(filenames[i], fd_in[i], buf_in[i], fp_in[i]);
        fp_in[i] += PAGE_SIZE;
    }

    db_key_t min_key_prev;
    while (true) {
        size_t i = 0;
        while (i < 2 && (offset_in[i] - start_in[i]) / DB_PAIR_SIZE == sizes[i][height[i] - 1]) {
            // i-th file has been fully read
            i++;
        }
        if (i == 2) {
            // write output buffer to page in output file, if not empty
            if ((offset_out - fp_out) / DB_PAIR_SIZE < b) {
                aligned_pwrite(fd_out, buf_out, PAGE_SIZE, fp_out);
            } else {
                fence_keys.pop_back();
            }
            break;
        }

        // read minimum key from input buffers
        db_key_t min_key = ((pair<db_key_t, db_val_t> *) buf_in[i])[(offset_in[i] - fp_in[i] + PAGE_SIZE) / DB_PAIR_SIZE].first;
        db_val_t val = ((pair<db_key_t, db_val_t> *) buf_in[i])[(offset_in[i] - fp_in[i] + PAGE_SIZE) / DB_PAIR_SIZE].second;
        for (size_t j = i + 1; j < 2; j++) {
            if ((offset_in[j] - start_in[j]) / DB_PAIR_SIZE < sizes[j][height[j] - 1]) {
                // j-th file has NOT been fully read yet
                db_key_t key = ((pair<db_key_t, db_val_t> *) buf_in[j])[(offset_in[j] - fp_in[j] + PAGE_SIZE) / DB_PAIR_SIZE].first;
                if (key < min_key) {
                    min_key = key;
                    val = ((pair<db_key_t, db_val_t> *) buf_in[j])[(offset_in[j] - fp_in[j] + PAGE_SIZE) / DB_PAIR_SIZE].second;
                    i = j;
                }
            }
        }

        size_t lvl = stoi(filenames[0].substr(4, filenames[0].find(".2.bin")));

        // write minimum key to output buffer
        if ((offset_out - start_out == 0 || min_key != min_key_prev) &&
            (lvl < DB_MAX_LEVEL || val != DB_TOMBSTONE)) {
            ((pair<db_key_t, db_val_t> *) buf_out)[(offset_out - fp_out) / DB_PAIR_SIZE] = make_pair(min_key, val);
            offset_out += DB_PAIR_SIZE;
        }
        offset_in[i] += DB_PAIR_SIZE;

        min_key_prev = min_key;

        if ((offset_in[i] - fp_in[i] + PAGE_SIZE) / DB_PAIR_SIZE == b &&
            (offset_in[i] - start_in[i]) / DB_PAIR_SIZE < sizes[i][height[i] - 1]) {
            // read page in input file to input buffer
            this->bpread(filenames[i], fd_in[i], buf_in[i], fp_in[i]);
            fp_in[i] += PAGE_SIZE;
        }

        if ((offset_out - fp_out) / DB_PAIR_SIZE == b) {
            // write output buffer to page in output file
            aligned_pwrite(fd_out, buf_out, PAGE_SIZE, fp_out);
            fp_out += PAGE_SIZE;
            fence_keys.push_back(((pair<db_key_t, db_val_t> *) buf_out)[b - 1].first);
        }
    }

    size = offset_out / DB_PAIR_SIZE;

    // close input files
    for (size_t i = 0; i < 2; i++) {
        close(fd_in[i]);
        remove(filenames[i].c_str());
    }

    // close output file
    close(fd_out);
}

// second pass does B-tree reconstruction
void KeyValueStore::compact_files_second_pass(vector<string> filenames,
                                              size_t size,
                                              vector<db_key_t> fence_keys)
{
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    size_t height = ceil(log((double) size / b) / log(b + 1)) + 1;

    // populate levels 0 - (height - 2)
    vector<vector<db_key_t> > non_terminal_nodes;
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    size_t i = b - 1;
    ssize_t j_prev = 0;
    while (i < size) {
        for (ssize_t j = height - 2; j >= 0; j--) {
            if ((i + 1) % ((size_t) pow(b + 1, j) * b) == 0) {
                if (i < size - 1 || j < j_prev) {
                    non_terminal_nodes[height - 2 - j].push_back(fence_keys[i / b]);
                }
                j_prev = j;
                break;
            }
        }
        i += b;
    }

    // populate level (height - 1)
    const char *terminal_nodes = "temp";

    vector<size_t> sizes;
    for (size_t i = 0; i < height - 1; i++) {
        sizes.push_back(non_terminal_nodes[i].size());
    }
    sizes.push_back(size);
    sizes.push_back(0); // write null terminator

    size_t lvl = stoi(filenames[0].substr(4, filenames[0].find(".2.bin")));
    if (exists("sst." + to_string(lvl + 1) + ".1.bin")) {
        cout << "sst." + to_string(lvl + 1) + ".1.bin exists" << endl;
        const char *filename = ("sst." + to_string(lvl + 1) + ".2.bin").c_str();
        cout << "compacting into " << filename << endl;
        this->write_to_file(filename, sizes, non_terminal_nodes, terminal_nodes);

        remove("temp");

        vector<string> filenames = {
            "sst." + to_string(lvl + 1) + ".2.bin",
            "sst." + to_string(lvl + 1) + ".1.bin"
        }; // newest to oldest
        this->compact_files(filenames);
    } else {
        const char *filename = ("sst." + to_string(lvl + 1) + ".1.bin").c_str();
        cout << "compacting into " << filename << endl;
        this->write_to_file(filename, sizes, non_terminal_nodes, terminal_nodes);

        remove("temp");
    }
}

void KeyValueStore::serialize()
{
    size_t b = PAGE_SIZE / DB_PAIR_SIZE; // number of key-value pairs per page

    vector<pair<db_key_t, db_val_t> > pairs = this->memtable.scan(DB_KEY_MIN, DB_KEY_MAX);
    if (pairs.size() == 0) {
        return;
    }

    this->memtable = Memtable(this->memtable_size);

    size_t height = ceil(log((double) pairs.size() / b) / log(b + 1)) + 1;

    // populate levels 0 - (height - 2)
    vector<vector<db_key_t> > non_terminal_nodes;
    for (size_t i = 0; i < height - 1; i++) {
        non_terminal_nodes.push_back({});
    }

    size_t i = b - 1;
    ssize_t j_prev = 0;
    while (i < pairs.size()) {
        for (ssize_t j = height - 2; j >= 0; j--) {
            if ((i + 1) % ((size_t) pow(b + 1, j) * b) == 0) {
                if (i < pairs.size() - 1 || j < j_prev) {
                    non_terminal_nodes[height - 2 - j].push_back(pairs[i].first);
                }
                j_prev = j;
                break;
            }
        }
        i += b;
    }

    // populate level (height - 1)
    vector<pair<db_key_t, db_val_t> > terminal_nodes = pairs;
    
    vector<size_t> sizes;
    for (size_t i = 0; i < height - 1; i++) {
        sizes.push_back(non_terminal_nodes[i].size());
    }
    sizes.push_back(terminal_nodes.size());
    sizes.push_back(0); // write null terminator

    if (exists("sst.1.1.bin")) {
        const char *filename = "sst.1.2.bin";
        cout << "serializing into " << filename << endl;
        this->write_to_file(filename, sizes, non_terminal_nodes, terminal_nodes);

        vector<string> filenames = {"sst.1.2.bin", "sst.1.1.bin"}; // newest to oldest
        this->compact_files(filenames);
    } else {
        const char *filename = "sst.1.1.bin";
        cout << "serializing into " << filename << endl;
        this->write_to_file(filename, sizes, non_terminal_nodes, terminal_nodes);
    }
}

void KeyValueStore::delete_sst_files() {
    const std::string path = std::filesystem::current_path().string();
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const std::string& filename = entry.path().filename().string();
        if (filename.size() > 7 && filename.substr(0, 3) == "sst" && filename.substr(filename.size() - 4) == ".bin") {
            std::filesystem::remove(entry.path());
            std::cout << "Deleted file: " << filename << std::endl;
        }
    }
}

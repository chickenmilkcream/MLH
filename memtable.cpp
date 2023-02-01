#include <iostream>
#include <vector>
#include <utility>
#include <fstream>
#include "memtable.h"

using namespace std;


SST::SST(string name) {
    this->name = name;
}

vector<pair<db_key_t, db_val_t>> SST::scan(db_key_t min_key, db_key_t max_key) {
    // TODO

    return vector<pair<db_key_t, db_val_t>>();
}


AVLNode::AVLNode(db_key_t key, db_val_t val) {
    this->key = key;
    this->val = val;
    this->left = NULL;
    this->right = NULL;
    this->parent = NULL;
}

void AVLNode::put(db_key_t key, db_val_t val) {
    if (this->key > key) {
        if (!this->left) {
            this->left = new AVLNode(key, val);
        } else {
            (*this->left).put(key, val);
        }
    } else if (this->key < key) {
        if (!this->right) {
            this->right = new AVLNode(key, val);
        } else {
            (*this->right).put(key, val);
        }
    } else {
        this->val = val;
    }
}

db_val_t AVLNode::get(db_key_t key) {
    if (this->key > key) {
        return (*this->left).get(key);
    }
    if (this->key < key) {
        return (*this->right).get(key);
    }
    return this->val;
}

void AVLNode::scan(db_key_t min_key, db_key_t max_key, vector<pair<db_key_t, db_val_t>> &pairs) {
    if (this->key > min_key) {
        if (this->left) {
            (*this->left).scan(min_key, max_key, pairs);
        }
    }
    if (this->key >= min_key && this->key <= max_key) {
        pair<db_key_t, db_val_t> pair;
        pair = make_pair(this->key, this->val);
        pairs.push_back(pair);
    }
    if (this->key < max_key) {
        if (this->right) {
            (*this->right).scan(min_key, max_key, pairs);
        }
    }
}


AVLTree::AVLTree() {
    this->root = NULL;
}

void AVLTree::put(db_key_t key, db_val_t val) {
    if (!this->root) {
        this->root = new AVLNode(key, val);
    } else {
        (*this->root).put(key, val);
    }
}

db_val_t AVLTree::get(db_key_t key) {
    if (this->root) {
        if (this->root->key == key) {
            return this->root->val;
        }
        return (*this->root).get(key);
    }
    return 0;
}

vector<pair<db_key_t, db_val_t>> AVLTree::scan(db_key_t min_key, db_key_t max_key) {
    vector<pair<db_key_t, db_val_t>> pairs;
    if (this->root) {
        (*this->root).scan(min_key, max_key, pairs);
    }
    return pairs;
}

Memtable::Memtable(uint8_t memtable_size) {
    this->max_size = memtable_size;
    this->size = 0;
    this->tree = AVLTree();
}

void Memtable::put(db_key_t key, db_val_t val) {
    this->tree.put(key, val);
}

db_val_t Memtable::get(db_key_t key) {
    return this->tree.get(key);
}

vector<pair<db_key_t, db_val_t>> Memtable::scan(db_key_t min_key, db_key_t max_key) {
    return this->tree.scan(min_key, max_key);
}


int main() {
    // test cases
    Memtable mt = Memtable(108);
    mt.put(5, 1);
    mt.put(6, 3);
    mt.put(2, 8);
    mt.put(1, 9);
//    mt.put(1, 7);
//    mt.put(1, 7);
//    mt.put(1, 7);
    mt.put(9, 8);

    // cout << mt.get(5) << endl;
    // cout << mt.get(2) << endl;
    // cout << mt.get(1) << endl;
    // cout << mt.get(9) << endl;
    // cout << mt.get(6) << endl;

    for (int i = 69; i > 0; i--) {
        mt.put(i, i + 100);
    }

    vector<pair<db_key_t, db_val_t>> mt_vector = mt.scan(0, 999);
    for (pair<db_key_t, db_val_t> pair: mt_vector) {
        cout << pair.first << ", " << pair.second << endl;
    }
    cout << "size: " << mt_vector.size() << endl;


    std::ofstream outputFile("sst_1.bin", std::ios::binary);
    for (const auto &p: mt_vector) {
        outputFile.write((char *) &p, sizeof(p));
    }
    outputFile.close();


    vector<std::pair<int, int>> vec;
    ifstream inputFile("sst_1.bin", ios::binary);

    while (inputFile.good()) {
        pair < db_key_t, db_val_t > p;
        inputFile.read((char *) &p, sizeof(p));
        vec.emplace_back(p);
    }
    inputFile.close();

    for (const auto &p: vec) {
        cout << p.first << " " << p.second << endl;
    }

    return 0;

}
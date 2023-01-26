//
// Created by jason on 1/26/2023.
//

#ifndef CSC443_PA_NODE_H
#define CSC443_PA_NODE_H


#include <string>

class Node {
public:
    Node(int key, int val);

    int key;
    int val;
    Node *left;
    Node *right;
    int height;
};



#endif //CSC443_PA_NODE_H

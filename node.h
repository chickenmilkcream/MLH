//
// Created by jason on 1/26/2023.
//

#ifndef CSC443_PA_NODE_H
#define CSC443_PA_NODE_H
#include <iostream>


template <typename T>
class node{
public:
    T key;
    int height;
    node * left;
    node * right;
    node(T k, int val){
        height = 1;
        key = k;
        val = val;
        left = NULL;
        right = NULL;
    }
};


#endif //CSC443_PA_NODE_H

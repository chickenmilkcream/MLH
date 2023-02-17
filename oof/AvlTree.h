//
// Created by jason on 1/31/2023.
//

// #ifndef CSC443_PA_AVLTREE_H
// #define CSC443_PA_AVLTREE_H

#pragma once

class Node
{
public:
    int key;
    Node *left;
    Node *right;
    int height;
};

class AVLTree
{
public:
    AVLTree();
    ~AVLTree();
    void insert(int key);
    void preOrder();

private:
    int height(Node *N);
    int max(int a, int b);
    Node *newNode(int key);
    Node *rightRotate(Node *y);
    Node *leftRotate(Node *x);
    int getBalance(Node *N);
    Node *insert(Node *node, int key);
    void preOrder(Node *root);
    Node *root;
};

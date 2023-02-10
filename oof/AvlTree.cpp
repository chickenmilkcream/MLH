//
// Created by jason on 1/31/2023.
//

#include "AvlTree.h"



#include "AvlTree.h"
#include <iostream>
#include <cmath>

AVLTree::AVLTree() : root(NULL) {}

AVLTree::~AVLTree() {}

int AVLTree::height(Node *N) {
    if (N == NULL)
        return 0;
    return N->height;
}

int AVLTree::max(int a, int b) {
    return (a > b)? a : b;
}

Node* AVLTree::newNode(int key) {
    Node* node = new Node();
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return(node);
}

Node *AVLTree::rightRotate(Node *y) {
    Node *x = y->left;
    Node *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right))+1;
    x->height = max(height(x->left), height(x->right))+1;
    return x;
}

Node *AVLTree::leftRotate(Node *x) {
    Node *y = x->right;
    Node *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right))+1;
    y->height = max(height(y->left), height(y->right))+1;
    return y;
}

int AVLTree::getBalance(Node *N) {
    if (N == NULL)
        return 0;
    return height(N->left) - height(N->right);
}

Node* AVLTree::insert(Node* node, int key) {
    if (node == NULL)
        return(newNode(key));
    if (key < node->key)
        node->left = insert(node->left, key);
    else if (key > node->key)
        node->right = insert(node->right, key);
    else
        return node
    node->height = 1 + max(height(node->left),
                           height(node->right));
    int balance = getBalance(node);
    if (balance > 1 && key < node->left->key)
        return rightRotate(node);
    if (balance < -1 && key > node->right->key)
        return leftRotate(node);
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

void AVLTree::preOrder(Node *root) {
    if(root != NULL) {
        std::cout << root->key << " ";
        preOrder(root->left);
        preOrder(root->right);
    }
}

void AVLTree::insert(int key) {
    root = insert(root, key);
}

void AVLTree::preOrder() {
    preOrder(root);
}

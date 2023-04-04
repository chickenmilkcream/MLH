#include <iostream>
#include "BloomFilter.h"

int main() {
    BloomFilter filter(10000, 7);

    filter.insert("hello");
    filter.insert("world");

    std::cout << "hello: " << filter.contains("hello") << std::endl;
    std::cout << "world: " << filter.contains("world") << std::endl;
    std::cout << "test: " << filter.contains("test") << std::endl;

    return 0;
}
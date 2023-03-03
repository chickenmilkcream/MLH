#include <iostream>
#include <cassert>
#include "bp_directory.h"

int main(int argc, char *argv[])
{
    // Want the max size to be 10 key-value pairs with each pair taking up 16 bytes

    std::cout << "Running test cases for Phase 2 \n";
    std::cout << "--------------------------------------------------------------- \n";

    BPDirectory bpd = BPDirectory("clock", 3, 3, 100);

    for (auto entry = bpd.directory.begin(); entry != bpd.directory.end(); ++entry)
    {
        std::cout << entry->first << ": " << std::endl;
        entry->second.addPageFrame(1);
        entry->second.printList();
    }
}
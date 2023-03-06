#include <iostream>
#include <cassert>
#include "bp_directory.h"

int main(int argc, char *argv[])
{
    // Want the max size to be 10 key-value pairs with each pair taking up 16 bytes

    std::cout << "Running test cases for Phase 2 \n";
    std::cout << "--------------------------------------------------------------- \n";

    BPDirectory bpd = BPDirectory("clock", 3, 10, 100);

    int start = 0;
    for (auto entry = bpd.directory.begin(); entry != bpd.directory.end(); ++entry)
    {
        std::cout << "--------------------------------------------------------------- \n";
        std::cout << "The directory entry: " << entry->first << ": " << std::endl;
        entry->second->addPageFrame(start);
        entry->second->printList();
        start ++;
    }
    std::cout << "--------------------------------------------------------------- \n";
    std::cout << "GONNA EXTEND \n";
    
    bpd.extend_directory();

    bpd.directory["1111"]->addPageFrame(12);
    bpd.directory["1011"]->addPageFrame(100);

    for (auto entry = bpd.directory.begin(); entry != bpd.directory.end(); ++entry)
    {
        std::cout << "--------------------------------------------------------------- \n";
        std::cout << "The directory entry: " << entry->first << ": " << std::endl;
        entry->second->printList();
    }

    bpd.insert_page("sst1", 1); // This will get hashed to 1111
    bpd.directory["1111"]->printList();
}
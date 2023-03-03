#ifndef BP_PAGEFRAME_H
#define BP_PAGEFRAME_H

#include <cstdint>
#include <string>
#include <iostream>

using namespace std;

class PageFrame
{
public:
    PageFrame(int val);
    int value;
    PageFrame *next;
};

#endif

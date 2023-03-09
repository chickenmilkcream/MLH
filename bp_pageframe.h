#ifndef BP_PAGEFRAME_H
#define BP_PAGEFRAME_H

#include <cstdint>
#include <string>
#include <iostream>

using namespace std;

class PageFrame
{
public:
    PageFrame(int page_content, string sst_name, int page_number);
    string sst_name;
    int page_number;
    int page_content;
    PageFrame *next;
};

#endif

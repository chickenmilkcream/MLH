#include "bp_pageframe.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>

#include <fcntl.h>

PageFrame::PageFrame(int page_content, string sst_name, int page_number)
{
    this->page_content = page_content;
    this->sst_name = sst_name;
    this->page_number = page_number;
    next = nullptr;
}
#include "bp_pageframe.h"
#include <iostream>
#include <vector>
#include <utility>
#include <fcntl.h>
#include <unistd.h>

#include <fcntl.h>

PageFrame::PageFrame(int val)
{
    value = val;
    next = nullptr;
}